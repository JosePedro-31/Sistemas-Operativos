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

typedef struct ListaTarefas{
    int tempo;
    char prog[20];
    char* args[300];
    time_t tempo_total;
} taskList;

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
        
        // Escrever no ecrá que a tarefa foi recebida
        int task = 1;
        printf("TASK %d Received\n", task);
        task+=1;

        // registar o tempo através da função gettimeofday
        struct timeval tempoinit;
        gettimeofday(&tempoinit, NULL);
        time_t tempo_inicial = tempoinit.tv_sec;

        // separar os elementos do buffer usando a strtok para uma taskList
        taskList tarefa;
        char *token = strtok(buffer, " ");
        // converter o tempo para inteiro
        tarefa.tempo = atoi(token);
        token = strtok(NULL, " ");
        // copiar o prog para a struct
        strcpy(tarefa.prog, token); 
        token = strtok(NULL, " ");
        // copiar os args para a struct 
        int i = 0;
        // enquanto houver tokens
        while (token != NULL) {
            // copiar o token para a struct
            tarefa.args[i] = token;
            // ir buscar o próximo token
            token = strtok(NULL, " ");
            i++;
        }
        
        // criar um processo filho
        pid_t pid = fork();
        if (pid == 0) {
            // executar o prog com os args
            execvp(tarefa.prog, tarefa.args);
            // se o execvp falhar
            perror("Erro na execução do prog\n");
            _exit(1);
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
                // calcular o tempo que demorou a tarefa
                tarefa.tempo_total = tempo_final - tempo_inicial;
            }
        }    

    }

    return 0;
}