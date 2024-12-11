//Diego Miranda 2210996 
//Felipe Cancella 2210487
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> 

#define MEM_P 0x3000
#define MEM_N 0x4000

struct processo {
    char nome[30];
    int ini;
    int dur;
    //int tipo;
    int ind;
    pid_t pid;
} typedef Processo;
