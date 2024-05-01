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


    printf("Usage:\n");
    printf("Execute a function: ./client execute [time] -u '[prog] [arg-1] [arg-2] ...'\n");
    printf("Check all tasks: ./client status\n\n");


    if ((strcmp(argv[1], "execute") == 0) && (strcmp(argv[3], "-u") == 0)) {
        

        // criar a tarefa
        OngoingTask currentTask;

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

        printf("Args : %s\n", currentTask.args);

        // abrir o fifo SERVER para enviar informação para o servidor
        int fds = open(SERVER, O_WRONLY);
        if(fds == -1){
            perror("Erro na abertura do fifo fds (client side)\n");
            _exit(1);
        
        }

        // escrever no fifo a struct
        if( write(fds, &currentTask, sizeof(struct OngoingTask)) == -1){
            perror("Erro a escrever no fifo fds (client side)\n");
            _exit(1);
        }  
        
        // fechar o fifo
        close(fds);
	}
    

    //if ( strcmp(argv[1],"status") == 0 ) {
        
        //status();
    //}
    
    return 0;
}