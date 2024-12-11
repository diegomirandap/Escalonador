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


char* args[] = {};

//Valida se o processo RT a ser inserido colide com outro ou excede minuto
int validaProcessosRT(int tam, Processo* processo, Processo* rt_vetor) {
    int end_time = processo->ini + processo->dur;
    if (end_time > 60) 
        return 1;

    for (int i = 0; i < tam; i++) {
        int rt_end_time = rt_vetor[i].ini + rt_vetor[i].dur;

        if (processo->ind > rt_vetor[i].ind && processo->ini >= rt_vetor[i].ini && processo->ini <= rt_end_time - 1) 
            return 1;      
    }

    return 0;
}

//Realiza a divisao para a execucao do processo
void alocaProcesso(int* ind, Processo* comando, Processo* processo) {
    strcpy(comando->nome, processo->nome);
    comando->ind = processo->ind;
    comando->ini = processo->ini;
    comando->dur = processo->dur;
    comando->pid = fork();

    if (comando->pid == 0) {
        if (execvp(comando->nome, args) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
        }
    } 
    else if (comando->pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else {
        kill(comando->pid, SIGSTOP);
        (*ind)++;
    }
}

int comparaProcesso(const void* a, const void* b) {
    const Processo* Processo_a = (const Processo*)a;
    const Processo* Processo_b = (const Processo*)b;
    return Processo_a->ini - Processo_b->ini;
}


int main(void) {

    struct timeval t;
    int tempo, ticks = 0, rt_rodando = 0, rt_cont = 0, rr_rodando = 0,  rr_cont = 0 ;

    int segP = shmget(MEM_P, sizeof(Processo), IPC_CREAT | 0666);
    if (segP==0) {
        fprintf(stderr, "Erro ao alocar memoriaE1\n");
        exit(-2);
    }
    Processo* processo = (Processo*) shmat(segP, 0, 0);
    Processo rt_vetor[10], rr_vetor[10];

    int segN   = shmget(MEM_N, sizeof(int),  IPC_CREAT | 0666);
    if (segN==0) {
        fprintf(stderr, "Erro ao alocar memoriaE2\n");
        exit(-2);
    }
    int* num = (int*) shmat(segN, 0, 0);

    while (ticks < 120) {
        if (rt_cont + rr_cont != *num) { // se falta processo a ser inserido
            qsort(rt_vetor, rt_cont, sizeof(Processo), comparaProcesso);
            if (processo->ini != -1) {//Realtime
                if (validaProcessosRT(rt_cont, processo, rt_vetor) == 0) { //verifica o processo
                    alocaProcesso(&rt_cont, &rt_vetor[rt_cont], processo);
                    printf("Alocou RT: \t%s\n", processo->nome);
                }
                else 
                printf("Nao vai alocar RT: \t%s (colide com %s)\n",processo->nome, rt_vetor[rt_cont-1].nome);
            }
            else {//Robin
                alocaProcesso(&rr_cont, &rr_vetor[rr_cont], processo);
                printf("Alocou RR: \t%s\n", processo->nome);
            }
        }

        gettimeofday(&t, NULL);
        tempo = t.tv_sec % 60;
        printf("\nSegundos: \t%ds\n", tempo);

        // Tratando Processo REAL-TIME
        if (rt_cont > 0){
            for (int i = 0; i < rt_cont; i++) { // Checa cada Processoo RT existente
                // Condicao de inicio
                if (tempo == rt_vetor[i].ini) {
                    kill(rt_vetor[i].pid, SIGCONT);
                    rt_rodando = 1;
                    printf("Rodando RT \t%s\n", rt_vetor[i].nome);
                }
                // Condicao de parada
                if (tempo == (rt_vetor[i].ini + rt_vetor[i].dur) - 1) {
                    rt_rodando = 0;
                    kill(rt_vetor[i].pid, SIGSTOP);
                }
            }
            if(rt_rodando!=0){
                printf("Rodando RT\n");
            }
        }
        // Tratando Processoo ROUND-ROBIN
        if (rt_rodando == 0 && rr_cont > 0) {
            int rr_anterior = (rr_rodando - 1 + rr_cont) % rr_cont; // Circularmente anterior
            kill(rr_vetor[rr_anterior].pid, SIGSTOP);
            if (rr_anterior != rr_rodando){ // Se tem apenas um RR na lista, apenas para
                kill(rr_vetor[rr_rodando].pid, SIGCONT);
                printf("Rodando RR \t%s\n", rr_vetor[rr_rodando].nome);
            }
            rr_rodando = (rr_rodando + 1) % rr_cont; // Próximo Processoo RR
        }
        sleep(1);
        ticks++;
    }

    shmdt(processo);
    shmctl(segP, IPC_RMID, 0);

    shmdt(num);
    shmctl(segN, IPC_RMID, 0);

    return 0;
}






