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

    if ((strcmp(argv[1], "execute") == 0) && (strcmp(argv[3], "-u") == 0)) {
        
        // abrir o fifo SERVER para enviar informação para o servidor
        int fds = open(SERVER, O_WRONLY);
        if(fds == -1){
            perror("Erro na abertura do fifo fds (client side)\n");
            _exit(1);
        
        }

        // criar a tarefa
        OngoingTask currentTask;

        // Tipo de tarefa (0 - exec, 1 - status)
        currentTask.type = 0;

        // guardar o programa e os argumentos numa só string

        int i;
        int argsSize = 0;
        for(i = 4; i < argc; i++){
            argsSize += strlen(argv[i]) + 1;
        }

        char args[argsSize];
        strcpy(args, "");
        for(i = 4; i < argc; i++){
            strcat(args, argv[i]);
            strcat(args, " ");
        }
        strcat(args, "\0");

        // preencher a struct
        currentTask.argsSize = argc - 4;
        currentTask.time = atoi(argv[2]);
        currentTask.pid = getpid();
        strcpy(currentTask.prog, argv[4]);
        strcpy(currentTask.args, args);

        // escrever no fifo a struct
        if(write(fds, &currentTask, sizeof(struct OngoingTask)) == -1){
            perror("Erro a escrever no fifo fds (client side)\n");
            _exit(1);
        }  
        
        // fechar o fifo
        close(fds);
	}
    

    if ( strcmp(argv[1],"status") == 0 ) {
        
        // abrir o fifo SERVER para enviar informação para o servidor
        int fds = open(SERVER, O_WRONLY);
        if(fds == -1){
            perror("Erro na abertura do fifo fds (client side)\n");
            _exit(1);
        
        }

        OngoingTask currentTask;
        // Tipo de tarefa (0 - exec, 1 - status)
        currentTask.type = 1;
        
        if (write(fds, &currentTask, sizeof(OngoingTask)) == -1) {

            perror("Erro a escrever\n");
            _exit(1);
        }

        // fechar o fifo
        close(fds);

        // abrir o fifo CLIENT para receber informação do servidor
        int fdc = open(CLIENT, O_RDONLY);
        if(fdc == -1){
            perror("Erro na abertura do fifo fdc (client side)\n");
            _exit(1);
        }


        // ler do fifo as tarefas a executar
        OngoingTask currentTaskStatus;
        int bytes_read;
        printf("\nExecuting\n");
        while((bytes_read = read(fdc, &currentTaskStatus, sizeof(OngoingTask))) > 0) {
            if (bytes_read == -1) {
                perror("Erro a ler do fifo client\n");
                _exit(1);
            }

            if (currentTaskStatus.type == -1) {
                printf("No executing tasks\n");
                break;
            }
            printf("%s %s\n", currentTaskStatus.taskID, currentTaskStatus.prog);
        }
        
        // ler do fifo as tarefas terminadas
        printf("\nCompleted\n");
        FinishedTask endTask;
        while ((bytes_read = read(fdc, &endTask, sizeof(struct FinishedTask))) > 0) {
            if (bytes_read == -1) {
                perror("Erro a ler do fifo client\n");
                _exit(1);
            }

            printf("%s %s %ld ms\n", endTask.taskID, endTask.prog, endTask.exec_time);
        }

        // fechar o fifo
        close(fdc);
    }

    if (strcmp(argv[1], "stop") == 0) {
        
        // abrir o fifo SERVER para enviar informação para o servidor
        int fds = open(SERVER, O_WRONLY);
        if(fds == -1){
            perror("Erro na abertura do fifo fds (client side)\n");
            _exit(1);
        
        }

        OngoingTask currentTask;
        // Tipo de tarefa (0 -> exec, 1 -> status, 2 -> stop)
        currentTask.type = 2;
        
        if (write(fds, &currentTask, sizeof(OngoingTask)) == -1) {

            perror("Erro a escrever\n");
            _exit(1);
        }

        // fechar o fifo
        close(fds);
    }

    if (strcmp(argv[1], "done") == 0) {
        
        // abrir o fifo SERVER para enviar informação para o servidor
        int fds = open(SERVER, O_WRONLY);
        if(fds == -1){
            perror("Erro na abertura do fifo fds (client side)\n");
            _exit(1);
        
        }

        OngoingTask currentTask;
        currentTask.type = 3;
        
        if (write(fds, &currentTask, sizeof(OngoingTask)) == -1) {

            perror("Erro a escrever\n");
            _exit(1);
        }

        // fechar o fifo
        close(fds);
    }

    if (strcmp(argv[1], "execute") != 0 && strcmp(argv[1], "status") != 0 && strcmp(argv[1], "stop") != 0 && strcmp(argv[1], "done") != 0){
        printf("Invalid command\n");

        printf("\nUsage:\n");
        printf("Execute a function: ./client execute [time] -u '[prog] [arg-1] [arg-2] ...'\n");
        printf("Check all tasks: ./client status\n\n");

    }
    
    return 0;
}