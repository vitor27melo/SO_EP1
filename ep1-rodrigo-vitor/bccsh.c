#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

void _mkdir(char **entrada_parseada){
    // Chama a system call mkdir
    if(mkdir(entrada_parseada[1], 0777) == -1)
        printf("Falha ao criar diretório: %s\n", strerror(errno));
}

void _kill(char **entrada_parseada){
    // Chama a system call kill
    pid_t pid = atoi(entrada_parseada[2]);
    if(kill(pid, SIGKILL) == -1)
        printf("Falha em matar o processo: %s\n", strerror(errno));
}

void _ln(char **entrada_parseada){
    // Código para criar pasta
    printf("%s%s%s%s",entrada_parseada[0],entrada_parseada[1], entrada_parseada[2],entrada_parseada[3]);
    if(symlink(entrada_parseada[2],entrada_parseada[3]) == -1)
        printf("Falha em criar link simbólico: %s\n", strerror(errno));
}

int leEntrada(char *entrada){
    char *buf;
    buf = readline(" ");
    if(strlen(buf) > 0 ){
        add_history(buf);
        strcpy(entrada, buf);
        return 0;
    }else{
        return 1;
    }
}

void parseEntrada(char *entrada, char **entrada_parseada){
    int i;
    for (i=0; i<=4 ; i++){
        entrada_parseada[i] = strsep(&entrada, " ");
        if (entrada_parseada[i] == NULL)
            break;
    }
}

int executaEntrada(char **entrada_parseada){
    pid_t pid;
    if (!strcmp("mkdir",entrada_parseada[0])){
        _mkdir(entrada_parseada);
    }else if (!strcmp("kill",entrada_parseada[0]) && !strcmp("-9",entrada_parseada[1])){
        _kill(entrada_parseada);
    }else if (!strcmp("ln",entrada_parseada[0]) && !strcmp("-s",entrada_parseada[1])){
        _ln(entrada_parseada);
    }else{
        pid = fork();
        if (pid == -1){
            printf("Falha ao fazer fork\n");
            return 1;
        } else if (pid == 0){
            if (execvp(entrada_parseada[0], entrada_parseada) < 0)
                printf("O programa '%s' não pôde ser aberto.\n", entrada_parseada[0]);
        } else {
            // Espera o programa child terminar
            waitpid(-1,NULL,0);
            return 0;
        }
        exit(0);
    }
    return 0;
}



int main(){
    // Recupera o nome do usuário
    char *user = getenv("USER");
    char cwd[1024], entrada[1024], *entrada_parseada[5];
    while (1){
        // Recupera o diretório atual
        getcwd(cwd, sizeof(cwd));
        // Imprime o prompt do shell 
        printf("{%s@%s}",user,cwd);
        // Espera e lê uma entrada (comando)
        if (leEntrada(entrada))
            continue;
        // Separa os comandos e argumentos
        parseEntrada(entrada, entrada_parseada);
        // Interpreta o comando
        executaEntrada(entrada_parseada);
        
    };
    return 0;
}




