#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/*
 * Function Declaration of builtin shell commands.
 */

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
 *  List of builtin commands, fllowed by their corresponding functions.
 */
const char *builtin_str[]={
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **)={
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins(){
    return sizeof(builtin_str)/sizeof(char*);
}

/*
 * Builtin function impletementions.
 */

int lsh_cd(char **args){
    if(args[1]==NULL)
    {
        fprintf(stderr,"lsh:expected arguments to \"cd\"\n");
    }else{
        if(chdir(args[1])!=0){
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args){
    int i;
    printf("HANGSHENGEE's LSH\n");
    printf("Type program name and arguments, and hit enter. \n");
    printf("The flowing are built in:\n");
    for(i=0;i<lsh_num_builtins();i++){
        printf("  %s\n",builtin_str[i]);
    }
    printf("Use the man command for information on other programs. \n");
    return 1;
}

int lsh_exit(char **args){
    return 0;
}

void lsh_loop();
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);
int lsh_launch(char **args);

int main(int args , char **argv){
    // Load config files
    // Run command loop

    lsh_loop();
    //Perform any shutdown/cleanup
    return EXIT_SUCCESS;

}

void lsh_loop()
{
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line=lsh_read_line();
        args=lsh_split_line(line);
        status=lsh_execute(args);
        free(line);
        free(args);
    }while(status);
}

#define LSH_RL_BUFFSIZE 1024

char *lsh_read_line(void){
   char *line = NULL ;
   size_t bufsize=0;
   if(getline(&line,&bufsize,stdin)==-1)
   {
       if(feof(stdin))
       {
           exit(EXIT_SUCCESS);
       }else{
           perror("readline");
           exit(EXIT_FAILURE);
       }           

    }
   return line;
}


#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a" 

char **lsh_split_line(char *line)
{
    int bufsize=LSH_TOK_BUFSIZE,position=0;
    char **tokens=(char **)malloc(sizeof(char *)*bufsize);
    char *token;

    if(!token){
        fprintf(stderr,"lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token=strtok(line,LSH_TOK_DELIM);
    while(token!=NULL){
        tokens[position]=token;
        position++;
        if(position>=bufsize){
            bufsize+=LSH_TOK_BUFSIZE;
            tokens=(char **)realloc(tokens,bufsize*sizeof(char *));
            if(!token){
                fprintf(stderr,"lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token=strtok(NULL,LSH_TOK_DELIM);
    }
    tokens[position]=NULL;
    return tokens;

} 

int lsh_execute(char **args)
{
    int i;
    if(args[0]==NULL){
        // an empty command was entered.
        return 1;
    }
    for(i=0;i<lsh_num_builtins();i++){
        if(strcmp(args[0],builtin_str[i])==0){
            return (*builtin_func[i])(args);
            
        }
    }
    return lsh_launch(args); 
}


int lsh_launch(char **args)
{
    pid_t pid,wpid;
    int status;
    pid=fork();
    if(pid==0)
    {
        //Child process
        if(execvp(args[0],args)==-1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }else if(pid<0){
        perror("lsh");
    }else{
        // Parent process
        do{
            wpid=waitpid(pid,&status,WUNTRACED);
        }while(!WIFEXITED(status)&&!WIFSIGNALED(status));
    }
    return 1;
}
