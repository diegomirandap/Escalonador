//Diego Miranda 2210996 
//Felipe Cancella 2210487
#include "auxiliar.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>


int main(void) {

    FILE* file;

    // Abre o arquivo "exec.txt" para leitura
    if (!(file = fopen("exec.txt", "r"))) {
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        exit(-1);
    }

    // Definindo parâmetros para o escalonador
    char* command = "./escalonador";
    char* args[] = {"./escalonador", NULL};

    // Aloca memória compartilhada para o objeto 'Processo'
    int segP = shmget(MEM_P, sizeof(Processo),  IPC_CREAT | 0666);
    if (segP == 0) {
        fprintf(stderr, "Erro ao alocar memoriaI1\n");
        exit(-2);
    }
    Processo* processo = (Processo*) shmat(segP, 0, 0);

    // Aloca memória compartilhada para o número de processos
    int segN = shmget(MEM_N, sizeof(int),  IPC_CREAT | 0666);
    if (segN == 0) {
        fprintf(stderr, "Erro ao alocar memoriaI2\n");
        exit(-2);
    }
    int* num = (int*) shmat(segN, 0, 0);

    char nome[20];
    int i, d, lido, lines = -1;
    pid_t pid = fork();
    // Processo filho: lê o arquivo e escreve na memória compartilhada
    if (pid == 0) {
        printf("aaa");
        while (!feof(file)) {
            lido = fscanf(file, "Run  %s I=%d D=%d\n", nome, &i, &d);
            lines++;

            // Constrói o caminho do executável
            strcpy(processo->nome, "./");
            strcat(processo->nome, nome);
            processo->ind = lines;
            processo->pid = -1;

            if (lido == 3) { // Realtime
            processo->ini = i;
            processo->dur = d;
            } 
            else { // Robin
            processo->ini = -1;
            processo->dur = -1;
            }

            sleep(1);
        }
    } 
    else // Processo pai: executa o escalonador
        execvp(command, args);
    printf("--%d--",lines);
    // Escreve o número de linhas no segmento de memória compartilhada
    *num = lines;

    // Fecha o arquivo
    fclose(file);

    // Desanexa os segmentos de memória compartilhada
    shmdt(processo);
    shmdt(num);

    // Aguarda indefinidamente
    while (1);

    return 0;
}