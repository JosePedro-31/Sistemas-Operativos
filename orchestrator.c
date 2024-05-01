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


int main(int argc, char *argv[]) {


    if((mkfifo(SERVER, 0666) == -1) && errno != EEXIST) {
        perror("Erro a criar o fifo server\n");
        return -1;
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

    OngoingTask currentTask;
    int task = 1;
    // ler do fifo
    int bytes_read;
    while(bytes_read = read(fds, &currentTask, sizeof(struct OngoingTask)) > 0) {
        
        if (bytes_read == -1) {
            perror("Erro a ler do fifo server\n");
            return -1;
        }

        printf("\n\nTASK %d Received\n", task);
        // criar um identificador para a tarefa
        snprintf(currentTask.taskID, sizeof(currentTask.taskID), "T%d", task);
        // incrementar o task
        task++;

        
        // registar o tempo através da função gettimeofday
        struct timeval starttime;
        gettimeofday(&starttime, NULL);
        time_t start_time = starttime.tv_usec;

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
        int fd_out = open(currentTask.taskID, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd_out == -1) {
            perror("Error opening file");
            return -1;
        }
    
        int res = dup2(fd_out, 1); // duplicate file descriptor fd_out to stdout
    
        if (res == -1) {
            perror("Error duplicating file descriptor");
            return -1;
        }

        // criar um processo filho
        pid_t pid = fork();
        if (pid == -1){
            perror("Erro a criar o filho\n");
            _exit(-1);
        }
        if (pid == 0) {
            // PROCESSO FILHO
            // executar o programa
            execvp(currentTask.prog, commands);
            // se o execvp falhar
            perror("Erro na execução do programa\n");
            _exit(-1);
        }

        else {
            // PROCESSO PAI
            // esperar que o processo filho termine

            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {

                close(fd_out); // close file descriptor

                int res = dup2(1, fd_out_original); // duplicate file descriptor fd_out to stdout

                if (res == -1) {
                    perror("Error duplicating file descriptor");
                    return -1;
                }

                close(fd_out_original); // close file descriptor

                // registar o tempo através da função gettimeofday
                struct timeval finishtime;
                gettimeofday(&finishtime, NULL);
                time_t finish_time = finishtime.tv_usec;

                FinishedTask endTask;
                // guardar o identificador da tarefa nas FinishedTask
                strcpy(endTask.taskID, currentTask.taskID);
                // calcular o tempo que demorou a tarefa
                endTask.exec_time = finish_time - start_time;
                // guardar o pid do processo
                endTask.pid = currentTask.pid;

                printf("\n\nFINISHED TASK:\n");
                printf("Task ID: %s\n", endTask.taskID);
                printf("PID: %d\n", endTask.pid);
                printf("Execution Time: %ld ms\n", endTask.exec_time);

                // escrever para um ficheiro "Tarefas" o identificador da tarefa e o tempo que demorou a ser executada
                int fd = open("tarefas.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
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
    // fechar o fifo
    close(fds);
    unlink(SERVER);

    return 0;
}