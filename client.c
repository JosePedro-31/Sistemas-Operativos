#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>


void execute (int tempo, char* prog, char* args) {

    // abrir o fifo cliente_servidor_fifo
    int cliente_servidor = open("cliente_servidor_fifo", O_WRONLY | O_TRUNC, 0666);
    
    if(cliente_servidor == -1){
		perror("Erro na abertura do fifo cliente_servidor (client side)\n");
		_exit(1);
    
    }
    // escrever para o buffer o tempo, prog e args para que o servidor possa executar o programa
    char buffer[1024];
    sprintf(buffer, "%d %s %s", tempo, prog, args);
    // escrever para o fifo
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

    if ( argc < 3 ) {
        printf("Usage:\n");
        printf('Execute a function: ./client execute [time] -u "[prog] [arg-1] [arg-2] ..."\n');
        printf("Check all tasks: ./client status\n");
        return 1;
    }

    if ((strcmp(argv[1], "execute") == 0) && (strcmp(argv[3], "-u") == 0)) {
        // TO DO
        execute(atoi(argv[2]), argv[4], argv[5]);
    }

    if ( strcmp(argv[1],"status") == 0 ) {
        // TO DO
        status();
    }

    return 0;
}