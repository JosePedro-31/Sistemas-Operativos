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

typedef struct tarefas{
    int tempo;
    char prog[20];
    char args[100];
} Tarefa;

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
        printf("Recebi: %s\n", buffer);
        close(servidor_cliente);

        // registar o tempo através da função gettimeofday
        struct timeval tempoinit;
        gettimeofday(&tempoinit, NULL);
        time_t tempo_inicial = tempoinit.tv_sec;

        // int tempo;
        // char prog[20];
        // char args[100];
        // sscanf(buffer, "%d %s %s", &tempo, prog, args);

        // separar o buffer usando o strtok
        char *tempo_str = strtok(buffer, " ");
        char *prog = strtok(NULL, " ");
        char *args = strtok(NULL, " ");

        // converter o tempo para inteiro
        int tempo = atoi(tempo_str);

        
        // criar um processo filho
        pid_t pid = fork();

        if (pid == 0) {
            // executar o programa
            execl(prog, args, NULL);
            _exit(1);
        }
    }

    return 0;
}