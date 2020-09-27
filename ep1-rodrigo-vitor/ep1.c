#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


#define DEBUG 0

void * ThreadAdd(void * a){
    int i;
    for (i = -32767; 1>0; i++){
        if(i == 32767)
            i = -32767;
    }

}

void FIFO(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count){
    clock_t start;
    printf("-----------First-Come First-Served Scheduling-----------\n");
}

void SRTN(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count){
    clock_t start;
    printf("-----------Shortest Remaining Time Next Scheduling-----------\n");
}

void RR(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count){
    clock_t start;
    printf("-----------Round Robin Scheduling-----------\n");
}


char ** parseNome(FILE *trace_file, int count){
    // Cria um array de tamanho count com os nomes dos processos
    char **nome;
    int i, j;
    char c;
    nome = malloc(count*sizeof(char *));
    for (i = 0; i < count; i++){
        nome[i] = malloc(31*sizeof(char));
    }
    fseek(trace_file, 0, SEEK_SET);
    i = 0;
    while(i < count){
        j = 0;
        while((c = fgetc(trace_file)) != ' '){
            nome[i][j] = c;
            j++;
        }
        nome[i][j] = '\0';
        while(fgetc(trace_file) != '\n');
        i++;
    }
    return nome;

}

int * parseInt(FILE *trace_file, int n_espacos, int count){
    // Cria um array de tamanho count com algum dos valores de tempo dos processos
    int *tempo;
    tempo = malloc(count*sizeof(int));
    fseek(trace_file, 0, SEEK_SET);
    char c, n_espacos_aux, aux[5];
    int j, i = 0;
    while(i < count){
        for(int y=0;y<5;y++)
            aux[y] = '\0';
        n_espacos_aux = 0;
        c = fgetc(trace_file);
        while(c != '\n'){
            c = fgetc(trace_file);
            if (c == ' ')
                n_espacos_aux++;

            if (n_espacos_aux == n_espacos){
                j = 0;
                while((c = fgetc(trace_file)) != ' ' && c != '\n'){
                    aux[j] = c;
                    j++;
                }
                n_espacos_aux++;
            }
        } 
        sscanf(aux, "%d", &tempo[i]);
        i++;
    }
    return (tempo);
}


int main(int argc, char *argv[]){
    FILE *trace_file;
    trace_file = fopen(argv[2],"rb");
    char c;
    int i = 0, count = 0;
    // Determina o número de linhas do arquivo
    for (c = getc(trace_file); c != EOF; c = getc(trace_file))
        if (c == '\n')
            count++;

    int *t0_proc_list = parseInt(trace_file, 1, count);
    if (DEBUG){
            printf("t0's:\n");
        for(i=0;i<count;i++)
            printf("%d\n",t0_proc_list[i]);
        printf("----------------------------\n");
    }
    int *dt_proc_list = parseInt(trace_file, 2, count);
    if (DEBUG){
        printf("dt's:\n");
        for(i=0;i<count;i++)
            printf("%d\n",dt_proc_list[i]);
        printf("----------------------------\n");
    }
    int *deadline_proc_list = parseInt(trace_file, 3, count);
    if (DEBUG){
        printf("deadlines:\n");
        for(i=0;i<count;i++)
            printf("%d\n",deadline_proc_list[i]);
        printf("----------------------------\n");
    }
    char **nome_proc_list = parseNome(trace_file, count);
    if (DEBUG){
        printf("nomes:\n");
        for(i=0;i<count;i++)
            printf("%s\n",nome_proc_list[i]);
        printf("----------------------------\n");
    }
    if(atoi(argv[1]) == 1)
        FIFO(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count);
    else if(atoi(argv[1]) == 2)
        SRTN(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count);
    else
        RR(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count);
    

    return 0;
}