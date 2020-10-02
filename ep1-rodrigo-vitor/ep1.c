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
            return arg;
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
    int **arg, *arg2, context_chg = 0;

    char linha_saida_file[50] = {0};
    
    arg = malloc((2*count) * sizeof(int));
    arg2 = malloc((2*count) * sizeof(int));

    start = clock();
    for(int i = 0; i < count; i++){
        context_chg++;
        arg2[i] = i;
        arg[i] = &arg2[i];
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
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n", nome_list[i]);
            fprintf(stderr, "\nProcesso finalizado: %s\n", linha_saida_file);
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
    printf("-----------Shorteste Remaining Time Next Scheduling-----------\n");
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    clock_t start;
    pthread_t tid[count];

    char linha_saida_file[50] = {0};

    double tempo_inicio, tempo_corrido, tempo_total, auxd, aux;
    double *remaining_time = malloc(count * sizeof(double));
    double *tempo_rodado = malloc(count * sizeof(double));
    int i, j, y, *finalizado, **arg, *arg2, programas_finalizados = 0, context_chg = 0;
    int *processo_criado = malloc(count * sizeof(int));
    finalizado = malloc(count * sizeof(int));
    for(i=0;i<count;i++){
        finalizado[i] = 0;
        killthread[i] = 0;
        processo_criado[i] = 0;
        remaining_time[i] = dt_list[i];
        pthread_mutex_lock(&mutex[i]);
    }

    arg = malloc((2*count) * sizeof(int));
    arg2 = malloc((2*count) * sizeof(int));

    i = 0;
    int processos_criados = 0;
    start = clock();
    while (programas_finalizados < count){
        if (i >= count) i = 0;
        if (finalizado[i] == 1){
            i++;
            continue;
        }
        if ((((double)clock() - start) / CLOCKS_PER_SEC) < t0_list[i]){
            i++;
            continue;
        }
        if (processo_criado[i] == 0){
            arg2[i]=i;
            arg[i] = &arg2[i];
            if (pthread_create(&tid[i], NULL, ThreadAdd, arg[i])) {
                printf("\n ERROR creating thread");
                exit(1);
            }
            processos_criados++;
            if (d) 
                fprintf(stderr, "\nChegou um processo no sistema: %s %d %d %d\n\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
            processo_criado[i] = 1;
        }
        

        //pular os processos já termiandos
        for(j=0;j<processos_criados;j++){
            if(finalizado[j] == 0)
                aux = remaining_time[j];
        }
        for(j=0;j<processos_criados;j++){
            if(finalizado[j] == 1) continue;
            if(remaining_time[j]<=aux) {
                aux=remaining_time[j]; 
                i=j;
            }
        }

        tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC;
        pthread_mutex_unlock(&mutex[i]);
        if (d)
            fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        context_chg++;
        tempo_total = ((double)clock() - start) / CLOCKS_PER_SEC;
        while ((tempo_total - tempo_inicio) < remaining_time[i]*2){

            // Verificar se não deu o t0 de algum processo ainda nao iniciado
            // Se der, locka o processo atual, ajusta o remaining time, seta o i para o que sera criado e dá um break
            if(i<=count-2)
                if((tempo_total/2 >= t0_list[i+1]) && processo_criado[i+1] == 0){
                    tempo_rodado[i] = (tempo_total - tempo_inicio)/2;
                    
                    break;
                }

            tempo_total = ((double)clock() - start) / CLOCKS_PER_SEC;
        }
        tempo_rodado[i] = (tempo_total - tempo_inicio)/2;
        remaining_time[i] = remaining_time[i] - tempo_rodado[i];
        pthread_mutex_lock(&mutex[i]);
        if (d)
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n", nome_list[i]);
        if(remaining_time[i] <= 0){
            killthread[i] = 1;
            pthread_mutex_unlock(&mutex[i]);
            if (pthread_join(tid[i], NULL))  {
                printf("\n ERROR joining thread");
                exit(1);
            }
            sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], tempo_total/2, tempo_rodado[i]);

            if (d)
                fprintf(stderr, "\nProcesso finalizado: %s\n", linha_saida_file);
            fputs(linha_saida_file, saida_file);

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

void RR(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    clock_t start;
    pthread_t tid[count];

    double tempo_inicio, tempo_corrido, tempo_final, auxd;
    double *tempo_rodado = malloc(count * sizeof(double));
    int i, *processo_criado,**arg, *arg2, programas_finalizados = 0, context_chg = 0;
    processo_criado = malloc(count * sizeof(int));
    for (i=0;i<count;i++) tempo_rodado[i]=0;
    char linha_saida_file[50] = {0};
    
    arg = malloc((2*count) * sizeof(int));
    arg2 = malloc((2*count) * sizeof(int));
    for(i = 0; i < count; i++){
        processo_criado[i] = 0;
        pthread_mutex_lock(&mutex[i]);
        killthread[i] = 0;
    }
    int *finalizado = malloc(count * sizeof(int));
    for (i=0;i<count;i++) finalizado[i]=0;
    // Quantum
    double quantum = 0.1;
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
        if (processo_criado[i] == 0){
            arg2[i]=i;
            arg[i] = &arg2[i];
            if (pthread_create(&tid[i], NULL, ThreadAdd, arg[i])) {
                printf("\n ERROR creating thread");
                exit(1);
            }
            if (d) 
                fprintf(stderr, "Chegou um processo no sistema: %s %d %d %d\n\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
            processo_criado[i] = 1;
        }

        tempo_inicio = ((double)clock() - start) / CLOCKS_PER_SEC;
        pthread_mutex_unlock(&mutex[i]);
        context_chg++;
        if (d) fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        
        if (programas_finalizados == count-1){
            usleep((dt_list[i] - tempo_rodado[i])*1000000);
            if (d) fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n\n", nome_list[i]);
            killthread[i] = 1;
            pthread_mutex_unlock(&mutex[i]);
            if (pthread_join(tid[i], NULL))  {
                printf("\n ERROR joining thread");
                exit(1);
            }   
            sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], (((double)clock() - start) / CLOCKS_PER_SEC), tempo_rodado[i] );
            fputs(linha_saida_file, saida_file);
            if (d) fprintf(stderr, "Processo finalizado: %s\n", linha_saida_file);
            break;
        }else
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
            fputs(linha_saida_file, saida_file);
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

    return 0;
}