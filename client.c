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


void execute (int tempo, char* prog, char* args[], int argsSize) {

    
    // abrir o fifo cliente_servidor_fifo
    int cliente_servidor = open("cliente_servidor_fifo", O_WRONLY | O_TRUNC, 0666);
    
    if(cliente_servidor == -1){
		perror("Erro na abertura do fifo cliente_servidor (client side)\n");
		_exit(1);
    
    }
    // criar um buffer para guardar o tempo, o prog e os args
    char buffer[300];
    // escrever no buffer o tempo, o prog e os args
    sprintf(buffer, "%d %s %s", tempo, prog, args[0]);
    for (int i = 1; i < argsSize; i++) {
        strcat(buffer, " ");
        strcat(buffer, args[i]);
    }
    // escrever no fifo
    write(cliente_servidor, buffer, strlen(buffer));
    // fechar o fifo
    close(cliente_servidor);
}


int main(int argc, char *argv[]) {

    
    // LIGAÇÃO CLIENTE-SERVIDOR
    if((mkfifo("cliente_servidor_fifo", 0666) == -1) && errno != EEXIST) {

        perror("Erro a criar o fifo (client side)\n");
        _exit(1);
    }

    printf("Usage:\n");
    printf("Execute a function: ./client execute [time] -u '[prog] [arg-1] [arg-2] ...'\n");
    printf("Check all tasks: ./client status\n");


    if ((strcmp(argv[1], "execute") == 0) && (strcmp(argv[3], "-u") == 0)) {
        
        // guardar todos os argumentos dentro de um array args
        /*
        char *args[argc-4];
        for (int i = 5; i < argc; i++) {
            args[i-5] = argv[i];
        }
        // executar a função execute
        execute(atoi(argv[2]), argv[4], args, argc-4);
        */
        char *commands[argc-5];
	    int N = 0;
	    for(int i=5; i < argc; i++){
		    commands[N] = strdup(argv[i]);
		    printf("command[%d] = %s\n", N, commands[N]);
		    N++;
	    }
        // passar o tempo, programa, argumentos e o tamanho do array args
        execvp(argv[4], commands);
    }

    if ( strcmp(argv[1],"status") == 0 ) {
        
        //status();
    }
    

    
    return 0;
}