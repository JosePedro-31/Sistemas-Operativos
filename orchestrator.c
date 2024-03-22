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

typedef struct OngoingTask{
    int tempo;
    char prog[20];
    char *args[300];
    char taskID[20];
} OngoingTask;


typedef struct FinishedTask{
    char taskID[20];
    time_t tempo_exec;
} FinishedTask;

int main(int argc, char *argv[]) {

    // LIGAÇÃO CLIENTE-SERVIDOR
    if((mkfifo("cliente_servidor_fifo", 0666) == -1) && errno != EEXIST) {

        perror("Erro a criar o fifo (server side)\n");
        _exit(1);
    }
        
    while (1) {

        // abrir o fifo cliente_servidor_fifo
        int servidor_cliente = open("cliente_servidor_fifo", O_RDONLY);

        if(servidor_cliente == -1){
            perror("Erro na abertura do fifo cliente_servidor (server side)\n");
            _exit(1);
        }
        char buffer[1024];
        read(servidor_cliente, buffer, 1024);
        close(servidor_cliente);
        
        // Escrever no ecrá que a tarefa foi recebida
        int task = 1;
        printf("TASK %d Received\n", task);

        // registar o tempo através da função gettimeofday
        struct timeval tempoinit;
        gettimeofday(&tempoinit, NULL);
        // converter o tempo para milisegundos
        time_t tempo_inicial = tempoinit.tv_sec;

        // separar os elementos do buffer usando a strtok para uma taskList
        OngoingTask currTask;
        char *token = strtok(buffer, " ");
        // converter o tempo para inteiro
        currTask.tempo = atoi(token);
        token = strtok(NULL, " ");
        // copiar o prog para a struct
        strcpy(currTask.prog, token); 
        token = strtok(NULL, " ");
        // copiar os args para a struct 
        int i = 0;
        // enquanto houver tokens
        while (token != NULL) {
            // copiar o token para a struct
            currTask.args[i] = token;
            // ir buscar o próximo token
            token = strtok(NULL, " ");
            i++;
        }
        currTask.args[i] = NULL;
        // criar um identificador para a tarefa
        snprintf(currTask.taskID, sizeof(currTask.taskID), "T%d", task);
        // incrementar o task
        task++;
        
        // criar um processo filho
        pid_t pid = fork();
        if (pid == 0) {
            // executar o prog com os args
            execvp(currTask.prog, currTask.args);
            // se o execvp falhar
            perror("Erro na execução do programa\n");
            _exit(-1);
        }
        else {
            // PROCESSO PAI
            // esperar que o processo filho termine
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                // registar o tempo através da função gettimeofday
                struct timeval tempofinal;
                gettimeofday(&tempofinal, NULL);
                time_t tempo_final = tempofinal.tv_sec;

                FinishedTask endTask;
                // guardar o identificador da tarefa nas FinishedTask
                strcpy(endTask.taskID, currTask.taskID);
                // calcular o tempo que demorou a tarefa
                endTask.tempo_exec = tempo_final - tempo_inicial;

                // escrever para um ficheiro "Tarefas" o identificador da tarefa e o tempo que demorou a ser executada
                int fd = open("Tarefas", O_WRONLY | O_APPEND | O_CREAT, 0666);
                if (fd == -1) {
                    perror("Erro na abertura do ficheiro Tarefas\n");
                    _exit(1);
                }
                // escrever o identificador da tarefa
                write(fd, endTask.taskID, sizeof(char));
                // escrever o tempo de execução da tarefa
                write(fd, &endTask.tempo_exec, sizeof(time_t));
                // fechar o descritor do ficheiro
                close(fd);
            }
        }    
    }

    return 0;
}