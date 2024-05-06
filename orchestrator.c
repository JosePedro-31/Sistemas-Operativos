#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "defs.h"


// Função para inicializar a fila
TaskQueue* initializeQueue() {
    TaskQueue* queue = (TaskQueue*)malloc(sizeof(TaskQueue));
    if (queue == NULL) {
        perror("Erro ao alocar memória para a fila");
        _exit(1);
    }
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

// Função para adicionar uma tarefa à fila
void enqueue(TaskQueue* queue, FinishedTask task) {
    FinishedTask* newTask = (FinishedTask*)malloc(sizeof(FinishedTask));
    if (newTask == NULL) {
        perror("Erro ao alocar memória para a tarefa");
        _exit(1);
    }
    *newTask = task;
    newTask->next = NULL;
    if (queue->rear == NULL) {
        queue->front = newTask;
    }
    else {
        queue->rear->next = newTask;
    }
    queue->rear = newTask;
}

// Função para retirar uma tarefa da fila
FinishedTask dequeue(TaskQueue* queue) {
    if (queue->front == NULL) {
        perror("Erro a retirar tarefa da fila");
        _exit(1);
    }
    FinishedTask task = *queue->front;
    FinishedTask* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return task;
}


void execute(TaskQueue* queue, OngoingTask currentTask, int task) {   

    printf("\n\nTASK %d Received\n", task);

    // criar um identificador para a tarefa
    snprintf(currentTask.taskID, sizeof(currentTask.taskID), "T%d", task);

    // registar o tempo através da função gettimeofday
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
    currentTask.start_time = starttime.tv_usec;
    

    // separar os argumentos da string args
    char *commands[currentTask.argsSize + 1];
    int i = 0;
    char *token = strtok(currentTask.args, " ");
    while (token != NULL) {
        commands[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
    }
    commands[i] = NULL;

    printf("Time: %d\n", currentTask.time);
    printf("PID: %d\n", currentTask.pid);
    printf("Task ID: %s\n", currentTask.taskID);
    printf("Program: %s\n", currentTask.prog);      

    int fd_out_original = dup(1);
    int fd_erro_original = dup(2);

    // criar um processo filho
    pid_t pid = fork();
    if (pid == -1){
        perror("Erro a criar o filho\n");
        _exit(1);
    }
    if (pid == 0) {
        // PROCESSO FILHO

        // redirecionar a entrada para uma pasta out com o nome da tarefa
        char out[25];
        sprintf(out, "tmp/%s", currentTask.taskID);

        int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0777); // open file for writing

        if (fd_out == -1) {
            perror("Erro a abrir o ficheiro fd_out\n");
            _exit(1);
        }

        int res = dup2(fd_out, 1); // duplicate file descriptor fd_out to stdout

        if (res == -1) {
            perror("Erro a duplicar o descriptor de ficheiro 1\n");
            _exit(1);
        }

        int res2 = dup2(fd_out, 2); // duplicate file descriptor fd_out to stderr

        if (res2 == -1) {
            perror("Erro a duplicar o descritor de ficheiro 2\n");
            _exit(1);
        }
        close(fd_out); // close file descriptor

        // executar o programa
        execvp(currentTask.prog, commands);
        // se o execvp falhar
        perror("Erro na execução do programa\n");
        _exit(1);
    }

    else {
        // PROCESSO PAI
        // esperar que o processo filho termine

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            // se o processo filho terminou normalmente

            int res = dup2(1, fd_out_original); // duplicate file descriptor fd_out to stdout

            if (res == -1) {
                perror("Erro a duplicar o descriptor de ficheiro 1\n");
                _exit(1);
            }

            int res2 = dup2(2, fd_erro_original); // duplicate file descriptor fd_out to stderr

            if (res2 == -1) {
                perror("Erro a duplicar o descritor de ficheiro 2\n");
                _exit(1);
            }

            close(fd_out_original); // close file descriptor

            // registar o tempo através da função gettimeofday
            struct timeval finishtime;
            gettimeofday(&finishtime, NULL);
            currentTask.finish_time = finishtime.tv_usec;

            FinishedTask endTask;    // 1 representa a função execute
            
            // guardar o identificador da tarefa nas FinishedTask
            strcpy(endTask.taskID, currentTask.taskID);
            // calcular o tempo que demorou a tarefa e converter para milisegundos
            endTask.exec_time = (currentTask.finish_time - currentTask.start_time)/1000;
            // guardar o pid do processo
            endTask.pid = currentTask.pid;
            strcpy(endTask.prog, currentTask.prog);

            // adicionar a tarefa à fila
            enqueue(queue, endTask);

            printf("\n\nFINISHED TASK:\n");
            printf("Task ID: %s\n", endTask.taskID);
            printf("PID: %d\n", endTask.pid);
            printf("Execution Time: %ld ms\n", endTask.exec_time);

            // escrever para um ficheiro "Tarefas" o identificador da tarefa e o tempo que demorou a ser executada
            int fd = open("tmp/tarefas.txt", O_WRONLY | O_APPEND | O_CREAT, 0777);
            if (fd == -1) {
                perror("Erro na abertura do ficheiro Tarefas\n");
                _exit(1);
            }
            // escrever o identificador da tarefa
            write(fd, &endTask.taskID, sizeof(char));
            // escrever o tempo de execução da tarefa
            write(fd, &endTask.exec_time, sizeof(time_t));
            // fechar o descritor do ficheiro
            close(fd);
        }
    }
}



// Função status vai imprimir as tarefas que estão na fila e as que estão a ser executadas
void status (TaskQueue* queue, OngoingTask currentTask) {
    
    
    printf("\n\nExecuting\n");
    // se a task estiver vazia, não há tarefas a serem executadas
    if (currentTask.prog == "") {
        printf("No tasks are being executed\n");
    }
    printf("%s %s\n", currentTask.taskID, currentTask.prog);

    // ler a fila até deixar de haver tarefas
    FinishedTask task;
    printf("\n\nFinished\n");
    while (queue->front != NULL) {
        task = dequeue(queue);
        printf("%s %s %ld ms\n", task.taskID, task.prog, task.exec_time);
    }
    
}


int main(int argc, char *argv[]) {


    if((mkfifo(SERVER, 0666) == -1) && errno != EEXIST) {
        perror("Erro a criar o fifo server\n");
        _exit(1);;
    }

    // abrir o fifo SERVER
    int fds = open(SERVER, O_RDONLY);
    if(fds == -1){
        perror("Erro na abertura do fifo server (server side)\n");
        _exit(1);
    }

    int fdp = open(SERVER, O_WRONLY);
    if(fdp == -1){
        perror("Erro na abertura do fifo server (server side)\n");
        _exit(1);
    }  

    TaskQueue* queue = initializeQueue();
    OngoingTask currentTask;

    // criar a tarefa
    int task = 1;

    // ler do fifo
    int bytes_read;
    while((bytes_read = read(fds, &currentTask, sizeof(struct OngoingTask))) > 0) {
        
        if (bytes_read == -1) {
            perror("Erro a ler do fifo server\n");
            _exit(1);
        }

        // caso o tempo seja -1, terminar o ciclo
        if (currentTask.time <= -1) {
            break;
        }

        if (currentTask.type == 0) {
            execute(queue, currentTask, task);
            task++;
        }

        if (currentTask.type == 1) {
            status(queue, currentTask);
        }

        if (currentTask.type == 2) {
            // parar a execução
            break;
        }

    }

    // fechar o fifo
    close(fds);
    unlink(SERVER);
    
    
    return 0;
}