#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main(int argc, char *argv[]) {

    // LIGAÇÃO CLIENTE-SERVIDOR
    if((mkfifo("cliente_servidor_fifo", 0666) == -1) && errno != EEXIST) {

        perror("Erro a criar o fifo (server side)\n");
        _exit(1);
    }

    while (1) {

        int servidor_cliente = open("cliente_servidor_fifo", O_RDONLY);
        if(servidor_cliente == -1){
            perror("Erro na abertura do fifo cliente_servidor (server side)\n");
            _exit(1);
        }
        char buffer[1024];
        read(servidor_cliente, buffer, 1024);
        printf("Recebi: %s\n", buffer);
        close(servidor_cliente);
    }

    return 0;
}