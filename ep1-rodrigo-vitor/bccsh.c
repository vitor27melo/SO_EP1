#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

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
    for (i=0; i<3 ; i++){
        entrada_parseada[i] = strsep(&entrada, " ");
        if (entrada_parseada[i] == NULL)
            break;
    }
}

int executaEntrada(char **entrada_parseada){
    pid_t pid;
    if (!strcmp("mkdir",entrada_parseada[0])){
        printf("banana");
    }else{
        pid = fork();
        if (pid == -1)
        if (pid == 0){
            execvp(entrada_parseada[0], entrada_parseada);
        }
        exit(0);
    }
    return 0;
}

int main(){
    // Recupera o nome do usuário
    char *user = getenv("USER");
    char cwd[1024], entrada[1024], *entrada_parseada[4];
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




