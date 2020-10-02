#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define DEBUG 0

pthread_mutex_t *mutex;

short int *killthread;
// Vetor para comunicacao entre a main thread e as thread filhas;
// apenas a main thread a modifica.
// Funciona assim, para a branch numero n, o vetor commthread[n] tem os numero possiveis
// 0 -> Branch deve continuar sua execucao
// 1 -> Branch deve se finalizar

void * ThreadAdd(void * arg){ 
    void *ret;
    int i, n = *((int *)arg);
    // printf("Entrou no processo %d\n",n);
    int count = -20000;
    while(1){
        switch (killthread[n]){
        case 0:
            pthread_mutex_lock(&mutex[n]);
            pthread_mutex_unlock(&mutex[n]);
            for(i=0;i<2;i++){
                if(count<20000)
                    count++;
                else
                    count = -20000;
            }
            break;
        case 1:  
            return ret;
            break;
        }
    }
}

void FIFO(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE* saida_file, short int d){
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    clock_t start;
    pthread_t tid[count];
    start = clock();

    double tempo_inicio, tempo_corrido, tempo_final, auxd;
    int **arg, context_chg = 0;

    char linha_saida_file[50] = {0};
    
    arg = malloc((2*count) * sizeof(int));

    start = clock();
    for(int i = 0; i < count; i++){
        context_chg++;
        arg[i] = &i;

        killthread[i] = 0;
        pthread_mutex_unlock(&mutex[i]);

        tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC;
        while (tempo_inicio < t0_list[i])
            tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC;
        
        if (pthread_create(&tid[i], NULL, ThreadAdd, arg[i])) {
            printf("\n ERROR creating thread");
            exit(1);
        }
        if(d){
            fprintf(stderr, "Chegou um processo no sistema: %s %d %d %d\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
            fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        }        
        
        auxd = (double)clock() / CLOCKS_PER_SEC;
        tempo_corrido = auxd - tempo_inicio;
        while (tempo_corrido < dt_list[i]){
            auxd = (double)clock() / CLOCKS_PER_SEC;
            tempo_corrido = auxd - tempo_inicio;
        }
        killthread[i] = 1;

        if (pthread_join(tid[i], NULL))  {
            printf("\n ERROR joining thread");
            exit(1);
        }
        tempo_final = ((double)(clock() - start)) / CLOCKS_PER_SEC;
        sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], tempo_final, tempo_corrido );
        if(d){
            fprintf(stderr, "Processo finalizado: %s\n", linha_saida_file);
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n", nome_list[i]);
        }
        fputs(linha_saida_file, saida_file);
        // free(args);
    }
    if(d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
    // free(commthread);
}

void SRTN(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    printf("-----------Shortest Remaining Time Next Scheduling-----------\n");
}

void RR(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
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
    FILE *trace_file, *saida_file = 0;
    trace_file = fopen(argv[2],"rb");
    saida_file = fopen(argv[3],"w");
    char c;
    short int d = 0;
    int i, count = 0;
    if(argc == 5) d = 1;
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
        FIFO(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);
    else if(atoi(argv[1]) == 2)
        SRTN(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);
    else
        RR(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);
    
    
    // int *arg, *arglist;
    // mutex = malloc(2 * sizeof(pthread_mutex_t));
    // arglist = malloc(2 * sizeof(int));
    // killthread = malloc(2 * sizeof(short int));
    // pthread_t tid[2];
    // for(i=0;i<2;i++){
    //     killthread[i] = 0;
    //     pthread_mutex_lock(&mutex[i]);
    // }
    // arg = malloc(sizeof(int));
    // for(i=0;i<2;i++){
    //     arglist[i] = i;
    //     arg = &arglist[i];
    //     if (pthread_create(&tid[i], NULL, ThreadAdd, arg)) {
    //         printf("\n ERROR creating thread");
    //         exit(1);
    //     }
    // }  

    // sleep(1);
    // pthread_mutex_unlock(&mutex[0]);
    // sleep(1);
    // pthread_mutex_lock(&mutex[0]);
    // sleep(1);
    // killthread[0] = 1;
    // pthread_mutex_unlock(&mutex[0]);
    // pthread_mutex_lock(&mutex[0]);
    // pthread_mutex_lock(&mutex[0]);
    // sleep(1);
    // pthread_mutex_unlock(&mutex[1]);
    // sleep(1);
    // pthread_mutex_lock(&mutex[1]);
    // sleep(1);
    // pthread_mutex_unlock(&mutex[0]);
    // sleep(1);
    // pthread_mutex_lock(&mutex[0]);
    // killthread[0] = 1;
    // killthread[1] = 1;
    // pthread_join(tid[0], NULL);

    return 0;
}