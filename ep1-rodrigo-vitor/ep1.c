#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define QUANTUM 0.5

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
    // Declarações de variáveis
    clock_t start;
    pthread_t tid[count];
    double tempo_inicio, tempo_corrido, tempo_final;
    int *arg, context_chg = 0;
    char linha_saida_file[60] = {0};

    // Alocações de memoria
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t)); 
    arg = malloc((count) * sizeof(int));

    // Início do scheduler
    start = clock();
    for (int i = 0; i < count; i++){
        arg[i] = i;
        killthread[i] = 0;

        // Espera o tempo de início do próximo processo, caso ele ainda não tenho passado
        while ((tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC) < t0_list[i]);

        // Cria o próximo processo
        if (pthread_create(&tid[i], NULL, ThreadAdd, &arg[i])) {
            printf("\n ERROR creating thread");
            exit(1);
        }

        // Libera o processo para executar suas operações reais
        pthread_mutex_unlock(&mutex[i]);
        context_chg++;
        
        // Realoca o valor do tempo_inicio, para melhor representar quanto tempo real a thread utilizou
        tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (d){
            fprintf(stderr, "Chegou um processo no sistema: %s %d %d %d\n\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
            fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        }

        // Suspende a main thread pelo tempo de execução do processo
        usleep(dt_list[i]*1000000);
        
        // Calcula o tempo real utilizado pela thread
        tempo_corrido = ((double)clock() / CLOCKS_PER_SEC) - tempo_inicio;
        
        // Ajusta a flag killthread[i] para informar ao processo que ele deve se terminar
        killthread[i] = 1;
        if (pthread_join(tid[i], NULL))  {
            printf("\n ERROR joining thread");
            exit(1);
        }

        // Calcula o tempo desde o início do scheduler em que o processo terminou
        tempo_final = ((double)(clock() - start)) / CLOCKS_PER_SEC;

        // Popula o arquivo de saída
        sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], tempo_final, tempo_corrido);
        fputs(linha_saida_file, saida_file);
        if (d){
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n", nome_list[i]);
            fprintf(stderr, "\nProcesso finalizado: %s\n", linha_saida_file);
        }
    }
    // Popula o arquivo de saída
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
    if (d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);

    fclose(saida_file);

    // Libera a memória alocada
    free(killthread);
    free(mutex);
    free(arg);
}

void SRTN(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    // Declarações de variáveis
    clock_t start;
    pthread_t tid[count];
    double tempo_inicio, tempo_total, aux, *remaining_time, *tempo_rodado;
    int i, j, *finalizado, *arg, *processo_criado, programas_finalizados = 0, context_chg = 0, processos_criados = 0, last_process = -1;
    char linha_saida_file[60] = {0};

    // Alocações de memoria
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    remaining_time = malloc(count * sizeof(double));
    tempo_rodado = malloc(count * sizeof(double));
    processo_criado = malloc(count * sizeof(int));
    finalizado = malloc(count * sizeof(int));
    arg = malloc((2*count) * sizeof(int));

    // Inicializa as variáveis auxiliares
    for( i = 0; i < count; i++){
        finalizado[i] = 0;
        killthread[i] = 0;
        processo_criado[i] = 0;
        remaining_time[i] = dt_list[i];
        pthread_mutex_lock(&mutex[i]);
    }

    // Início do scheduler
    i = 0; 
    start = clock();
    while (programas_finalizados < count){
        // O scheduler roda em loop até que todos os processos terminem
        if (i >= count) i = 0;

        // Verifica se um processo i já terminou, e caso positivo, itera o loop
        if (finalizado[i] == 1){
            i++;
            continue;
        }

        // Espera o tempo de início do próximo processo, caso ele ainda não tenho passado
        if ((((double)clock() - start) / CLOCKS_PER_SEC)/2 < t0_list[i]){
            i++;
            continue;
        }

        // Inicializa o processo, caso ele ainda não tenha sido inicializado
        if (processo_criado[i] == 0){
            arg[i] = i;
            if (pthread_create(&tid[i], NULL, ThreadAdd, &arg[i])) {
                printf("\n ERROR creating thread");
                exit(1);
            }
            processos_criados++;
            if (d) 
                fprintf(stderr, "\nChegou um processo no sistema: %s %d %d %d no tempo %lf\n\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i],(((double)clock() - start) / CLOCKS_PER_SEC)/2);
            processo_criado[i] = 1;
        }
        
        // Decide o processo que será alocado para a CPU baseado no tempo restante para finalização de todos os processos
        for (j = 0; j < processos_criados; j++){
            if(finalizado[j] == 0)
                aux = remaining_time[j];
        }
        for (j = 0; j < processos_criados; j++){
            if(finalizado[j] == 1) continue;
            if(remaining_time[j]<=aux) {
                aux=remaining_time[j]; 
                i=j;
            }
        }

        // Libera o processo para executar suas operações reais
        pthread_mutex_unlock(&mutex[i]);
        tempo_inicio = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (d)
            fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);

        // Só consideramos mudança de contexto quando o processo que vai rodar é diferente do que estava rodando anteriormente
        if (last_process != i) context_chg++;
        last_process = i;
        
        // Contagem de tempo 'ativa', roda o processo i até ele terminar ou outro processo com tempo restante menor ser criado
        while (((tempo_total = ((double)clock() - start) / CLOCKS_PER_SEC) - tempo_inicio) < remaining_time[i]*2){
           // Verifica se foi atingido o t0 do próximo processo
            if((i <= count -2) && (tempo_total/2 >= t0_list[i+1]) && processo_criado[i+1] == 0) break;
           
        }

        // Pausa o processo que estava em execução
        pthread_mutex_lock(&mutex[i]);
        if (d)
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n", nome_list[i]);

        // Calculo o tempo total de CPU que o processo i já utilizou até o momento
        // e quanto falta para ele terminar
        tempo_rodado[i] += (tempo_total - tempo_inicio)/2;
        remaining_time[i] -=  tempo_rodado[i];

        // Caso o processo i já tenha executado pelo tempo especificado em dt[i],
        // ajusta a flag killthread[i] para informar ao processo que ele deve se terminar
        if (remaining_time[i] <= 0){
            killthread[i] = 1;
            pthread_mutex_unlock(&mutex[i]);
            if (pthread_join(tid[i], NULL))  {
                printf("\n ERROR joining thread");
                exit(1);
            }
            // Popula o arquivo de saída
            sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], tempo_total/2, tempo_rodado[i]);
            fputs(linha_saida_file, saida_file);
            if (d)
                fprintf(stderr, "\nProcesso finalizado: %s\n", linha_saida_file);
            
            // Ajusta as variáveis auxiliares
            programas_finalizados++;
            finalizado[i] = 1;
        }
        i++;
    }
    // Popula o arquivo de saída
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
    if (d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);

    fclose(saida_file);

    // Libera a memória alocada
    free(killthread);
    free(mutex);
    free(remaining_time);
    free(tempo_rodado);
    free(processo_criado);
    free(finalizado);
    free(arg);
}

void RR(int *t0_list, int *dt_list, int *deadline_list, char **nome_list, int count, FILE *saida_file, short int d){
    // Declarações de variáveis
    clock_t start;
    pthread_t tid[count];
    double tempo_inicio, *tempo_rodado;
    int i, *processo_criado, *arg, *finalizado, programas_finalizados = 0, context_chg = 0;
    char linha_saida_file[60] = {0};

    // Alocações de memoria
    killthread = malloc(count * sizeof(short int));
    mutex = malloc(count * sizeof(pthread_mutex_t));
    processo_criado = malloc(count * sizeof(int));
    tempo_rodado = malloc(count * sizeof(double));
    arg = malloc(count * sizeof(int));
    finalizado = malloc(count * sizeof(int));
    
    // Inicializa as variáveis auxiliares
    for (i = 0; i < count; i++){
        killthread[i] = 0;
        tempo_rodado[i] = 0;
        finalizado[i] = 0;
        processo_criado[i] = 0;
        pthread_mutex_lock(&mutex[i]);
    }
    
    // Início do scheduler
    i = 0;
    start = clock();
    while (programas_finalizados < count){
        // O scheduler roda em loop até que todos os processos terminem
        if (i >= count) i = 0;

        // Verifica se um processo i já terminou, e caso positivo, itera o loop
        if (finalizado[i] == 1){
            i++;
            continue;
        }tempo_inicio = ((double)clock() - start) / CLOCKS_PER_SEC;

        // Espera o tempo de início do próximo processo, caso ele ainda não tenho passado
        if ((((double)clock() - start) / CLOCKS_PER_SEC) < t0_list[i]){
            i++;
            continue;
        }

        // Inicializa o processo, caso ele ainda não tenha sido inicializado
        if (processo_criado[i] == 0){
            arg[i] = i;
            if (pthread_create(&tid[i], NULL, ThreadAdd, &arg[i])) {
                printf("\n ERROR creating thread");
                exit(1);
            }
            if (d) 
                fprintf(stderr, "Chegou um processo no sistema: %s %d %d %d\n\n", nome_list[i], t0_list[i], dt_list[i], deadline_list[i]);
            processo_criado[i] = 1;
        }

        // Libera o processo para executar suas operações reais
        pthread_mutex_unlock(&mutex[i]);
        tempo_inicio = ((double)clock() - start) / CLOCKS_PER_SEC;
        context_chg++;
        if (d) 
            fprintf(stderr, "O processo '%s' comecou a usar a CPU.\n", nome_list[i]);
        
        // Verifica se o processo a ser executado é o último na fila de processos,
        // nesse caso, ele roda de forma ininterrupta até terminar
        if (programas_finalizados == count - 1){
            usleep((dt_list[i] - tempo_rodado[i])*1000000);
        }else
            usleep(QUANTUM*1000000);

        // Pausa o processo que estava em execução
        pthread_mutex_lock(&mutex[i]);
        if (d)
            fprintf(stderr, "O processo '%s' deixou de usar a CPU.\n\n", nome_list[i]);

        // Calula o tempo total de execução de um processo
        tempo_rodado[i] += ((double)clock() / CLOCKS_PER_SEC) - tempo_inicio;

        // Caso o processo i já tenha executado pelo tempo especificado em dt[i],
        // ajusta a flag killthread[i] para informar ao processo que ele deve se terminar
        if (tempo_rodado[i] >= dt_list[i]){
            killthread[i] = 1;
            pthread_mutex_unlock(&mutex[i]);
            if (pthread_join(tid[i], NULL))  {
                printf("\n ERROR joining thread");
                exit(1);
            }   

            // Popula o arquivo de saída
            sprintf(linha_saida_file, "%s %lf %lf\n", nome_list[i], (((double)clock() - start) / CLOCKS_PER_SEC), tempo_rodado[i] );
            fputs(linha_saida_file, saida_file);
            if (d) 
                fprintf(stderr, "Processo finalizado: %s\n", linha_saida_file);

            // Ajusta as variáveis auxiliares
            programas_finalizados++;
            finalizado[i] = 1;
        }
        i++;
    }
    // Popula o arquivo de saída
    sprintf(linha_saida_file, "%d", context_chg);
    fputs(linha_saida_file, saida_file);
    if (d)
        fprintf(stderr, "No total, aconteceram %d mudancas de contexto.\n", context_chg);

    fclose(saida_file);

    // Libera a memória alocada
    free(killthread);
    free(mutex);
    free(processo_criado);
    free(tempo_rodado);
    free(arg);
    free(finalizado);
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
    while (i < count){
        j = 0;
        while ((c = fgetc(trace_file)) != ' '){
            nome[i][j] = c;
            j++;
        }
        nome[i][j] = '\0';
        while (fgetc(trace_file) != '\n');
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
    while (i < count){
        for (int y = 0; y < 5; y++)
            aux[y] = '\0';
        n_espacos_aux = 0;
        c = fgetc(trace_file);
        while (c != '\n'){
            c = fgetc(trace_file);
            if (c == ' ')
                n_espacos_aux++;

            if (n_espacos_aux == n_espacos){
                j = 0;
                while ((c = fgetc(trace_file)) != ' ' && c != '\n'){
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
    int count = 0;

    if(argc == 5) d = 1;

    // Determina o número de linhas do arquivo
    for (c = getc(trace_file); c != EOF; c = getc(trace_file))
        if (c == '\n')
            count++;

    int *t0_proc_list = parseInt(trace_file, 1, count);
    
    int *dt_proc_list = parseInt(trace_file, 2, count);
    
    int *deadline_proc_list = parseInt(trace_file, 3, count);
    
    char **nome_proc_list = parseNome(trace_file, count);

    if(atoi(argv[1]) == 1)
        FIFO(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);
    else if(atoi(argv[1]) == 2)
        SRTN(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);
    else
        RR(t0_proc_list, dt_proc_list, deadline_proc_list, nome_proc_list, count, saida_file, d);

    fclose(trace_file);
    free(t0_proc_list);
    free(dt_proc_list);
    free(deadline_proc_list);
    for(int i = 0; i < count; i++)
        free(nome_proc_list[i]);
    free(nome_proc_list);

    return 0;
}