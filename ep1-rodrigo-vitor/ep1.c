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
    int count = -20000;
    while(1){
        switch (killthread[n]){
        case 0:
            pthread_mutex_lock(&mutex[n]);
            pthread_mutex_unlock(&mutex[n]);
            for(i=0;i<5;i++){
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
        
        // "Sleep"
        // Aqui, nós multiplicamos o dt_list[n] por 2, pois como serão duas threads rodando,
        // o número de clocks que o processo utiliza será aproximadamente o dobro.
        // while (tempo_corrido < dt_list[i]*2){
        //     auxd = (double)clock() / CLOCKS_PER_SEC;
        //     tempo_corrido = auxd - tempo_inicio;
        // }
        usleep(dt_list[i]*1000000);
        
        auxd = (double)clock() / CLOCKS_PER_SEC;
        tempo_corrido = auxd - tempo_inicio;
        
        killthread[i] = 1;
        if (pthread_join(tid[i], NULL))  {
            printf("\n ERROR joining thread");
            exit(1);
        }
        tempo_final = ((double)(clock() - start)) / CLOCKS_PER_SEC;
        sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], tempo_final, tempo_corrido);
        if(d){
            fprintf(stderr, "Processo finalizado: %s", linha_saida_file);
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n\n", nome_list[i]);
        }
        fputs(linha_saida_file, saida_file);
        // free(args);
    }
    if(d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
}

void SRTN(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    printf("-----------Shortest Remaining Time Next Scheduling-----------\n");
}

void RR(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    printf("-----------Round Robin Scheduling-----------\n");
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    clock_t start;
    pthread_t tid[count];

    double tempo_inicio, tempo_corrido, tempo_final, auxd;
    double *tempo_rodado = malloc(count * sizeof(double));
    int i, **arg, *arg2, programas_finalizados = 0, context_chg = 0;

    for (i=0;i<count;i++) tempo_rodado[i]=0;
    char linha_saida_file[50] = {0};
    
    arg = malloc((2*count) * sizeof(int));
    arg2 = malloc((2*count) * sizeof(int));
    for(i = 0; i < count; i++){
        pthread_mutex_lock(&mutex[i]);
        arg2[i]=i;
        killthread[i] = 0;
        arg[i] = &arg2[i];
        if (pthread_create(&tid[i], NULL, ThreadAdd, arg[i])) {
            printf("\n ERROR creating thread");
            exit(1);
        }
        if (d) 
            fprintf(stderr, "Chegou um processo no sistema: %s %d %d %d\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
    }
    int *finalizado = malloc(count * sizeof(int));
    for (i=0;i<count;i++) finalizado[i]=0;
    // Quantum
    double quantum = 0.5;
    i = 0;
    start = clock();
    while(programas_finalizados < count){
        if (i == count) i = 0;
        if (finalizado[i] == 1){
            i++;
            continue;
        }
        if ((((double)clock() - start) / CLOCKS_PER_SEC) < t0_list[i]){
            i++;
            continue;
        }


        tempo_inicio = ((double)clock() - start) / CLOCKS_PER_SEC;
        context_chg++;
        pthread_mutex_unlock(&mutex[i]);
        if (d) fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        // auxd = (double)clock() / CLOCKS_PER_SEC;
        // tempo_corrido = auxd - tempo_inicio;
        // while (tempo_corrido < quantum*2){
        //     auxd = (double)clock() / CLOCKS_PER_SEC;
        //     tempo_corrido = auxd - tempo_inicio;
        // }
        // tempo_corrido = tempo_corrido/2;
        usleep(quantum*1000000);

        pthread_mutex_lock(&mutex[i]);

        auxd = (double)clock() / CLOCKS_PER_SEC;
        tempo_corrido = auxd - tempo_inicio;

        if (d) fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n\n", nome_list[i]);
        tempo_rodado[i] += tempo_corrido;
        if(tempo_rodado[i] >= dt_list[i]){
            killthread[i] = 1;
            pthread_mutex_unlock(&mutex[i]);
            if (pthread_join(tid[i], NULL))  {
                printf("\n ERROR joining thread");
                exit(1);
            }   

            sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], (((double)clock() - start) / CLOCKS_PER_SEC), tempo_rodado[i] );
            if (d) fprintf(stderr, "Processo finalizado: %s\n", linha_saida_file);
            programas_finalizados++;
            finalizado[i] = 1;
            
        }

        i++;
    }
    if(d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
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

    // double auxd, tempo_corrido, tempo_inicio;
    // clock_t start;

    // start = clock();

    // tempo_inicio = ((double)clock() - start) / CLOCKS_PER_SEC;

    // auxd = (double)clock() / CLOCKS_PER_SEC;
    // tempo_corrido = auxd - tempo_inicio;
    // while (tempo_corrido < 5){
    //     // printf("-");
    //     auxd = (double)clock() / CLOCKS_PER_SEC;
    //     tempo_corrido = auxd - tempo_inicio;
    // }

    // printf("tempo inicio: %lf\ntempo_corrido: %lf",tempo_inicio,tempo_corrido);

    return 0;
}