#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
/// description: Takes a line and splits it into args similar to how argc
///              and argv work in main
/// line:        The line being split up. Will be mangled after completion
///              of the function<
/// args:        a pointer to an array of strings that will be filled and
///              allocated with the args from the line
/// num_args:    a point to an integer for the number of arguments in args
/// return:      returns 0 on success, and -1 on failure
int lexer(char *line, char ***args, int *num_args){
    *num_args = 0;
    // count number of args
    char *l = strdup(line);
    if(l == NULL){
        return -1;
    }
    char *token = strtok(l, " \t\n");
    while(token != NULL){
        (*num_args)++;
        token = strtok(NULL, " \t\n");
    }
    free(l);
    // split line into args
    *args = malloc(sizeof(char **) * *num_args);
    *num_args = 0;
    token = strtok(line, " \t\n");
    while(token != NULL){
        char *token_copy = strdup(token);
        if(token_copy == NULL){
            return -1;
        }
        (*args)[(*num_args)++] = token_copy;
        token = strtok(NULL, " \t\n");
    }
    return 0;
}
int lexer2(char *line, char ***args, int *num_args){
    *num_args = 0;
    // count number of args
    char *l = strdup(line);
    if(l == NULL){
        return -1;
    }
    char *token = strtok(l, ";");
    while(token != NULL){
        (*num_args)++;
        token = strtok(NULL, ";");
    }
    free(l);
    // split line into args
    *args = malloc(sizeof(char **) * *num_args);
    *num_args = 0;
    token = strtok(line, ";");
    while(token != NULL){
        char *token_copy = strdup(token);
        if(token_copy == NULL){
            return -1;
        }
        (*args)[(*num_args)++] = token_copy;
        token = strtok(NULL, ";");
    }
    return 0;
}
void exit_smash(){
    exit(0);
}
void pwd(){
    char error_message[30] = "An error has occurred\n";
    char *buffer = malloc(256);
    if(getcwd(buffer, 256)==NULL){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
    }else{
        printf("%s\n",buffer);
    }
}
void cd(char *dir){
    char error_message[30] = "An error has occurred\n";
    if(chdir(dir)!=0){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}
int checkrdr(char **cmdargs, int num){
    for(int i=0;i<num;i++){
        if(!strcmp(cmdargs[i],">")){
                if(i != num - 2 || i == 0){
                    return -1;
                }
                return 1;
            }
    }
    return 0;    
}
void excmd(char **cmdargs, int num, int redir){
    char error_message[30] = "An error has occurred\n";
    //for(int i=0;i<num;i++)
    int nxti = 0;
    for(int i=0;i<num;i++){
        if(!strcmp(cmdargs[i],"|")){
            nxti = i+1;
            cmdargs[i]=NULL;
        }
    }
    //printf("%d\n",nxti);
    int pipefd[2];
    if(nxti != 0){
        pipe(pipefd);
    }
    char *myargs[num+1];
    if(!redir){
        for(int i=0;i<num;i++){
            myargs[i] = cmdargs[i];
        }
        myargs[num] = NULL;
    }else{
        for(int i=0;i<num-2;i++){
            myargs[i] = cmdargs[i];
        }
        myargs[num-2] = NULL;
    }

    int rc = fork();
    if(rc == 0){
        if(nxti != 0){
            int pid = fork();
            if (pid == 0){
      // child gets here and handles "grep bob"
      // replace standard input with input part of pipe
            
            if(redir){
            int newfd;
            if ((newfd = open(cmdargs[num-1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		        write(STDERR_FILENO, error_message, strlen(error_message));	/* open failed */
		        exit(1);
	        }
            dup2(newfd, 1);
            close(newfd);
        }
      // execute grep   
      int exec_rc = execv(myargs[nxti], myargs);
      if(exec_rc == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
    }
  else{
      // parent gets here and handles "cat scores"
      // replace standard output with output part of pipe
      dup2(pipefd[1], 1);
      // close unused innput half of pipe
      //wait(NULL);
      close(pipefd[0]);

      // execute cat
      int exec_rc = execv(myargs[0], myargs);
      if(exec_rc == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
 
    }
    //return 0;
        }else{
        //inside child process
        if(redir){
            int newfd;
            if ((newfd = open(cmdargs[num-1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		        write(STDERR_FILENO, error_message, strlen(error_message));	/* open failed */
		        exit(1);
	        }
            dup2(newfd, 1);
            close(newfd);
        }
        int exec_rc = execv(myargs[0], myargs);
        if(exec_rc == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
        printf("Done with execv\n");
        }
        
    }
    else{
        //parent process executes here
        //sleep(1);
        //use RC to wait for child process
        //int wait_rc = 
        wait(NULL);
        }
}
 
void execcmd(char **cmdargs,int num){
    char error_message[30] = "An error has occurred\n";
    //int err = 0;   
        if(!num){
            return;
        }
        if(!strcmp(cmdargs[0],"loop")&& num > 2){
            int lnum = atoi(cmdargs[1]);
            char **cmdarg= malloc(sizeof(char**)*(num-2));
            for(int k=2;k<num;k++){
                cmdarg[k-2] = malloc(100);
                strcpy(cmdarg[k-2],cmdargs[k]);
            }
            for(int k=0;k<lnum;k++){
                execcmd(cmdarg,num-2);
            }
            return;
        }
        int rdir = checkrdr(cmdargs,num);
        if(rdir==-1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }
        if(!strcmp(cmdargs[0],"exit")){
            if(num!=1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }
            exit_smash();
        }
        else if(!strcmp(cmdargs[0],"pwd")){
            if(num!=1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }
            pwd();
        }
        else if(!strcmp(cmdargs[0],"cd")){
            if(num!=2){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }
            cd(cmdargs[1]);
        }else{
            excmd(cmdargs,num, rdir);
        }
}
void mulcmd(char **cmd, int cnum){
    //char error_message[30] = "An error has occurred\n";
    int num;
    char **cmdargs;
    for(int i=0;i<cnum;i++){
        lexer(cmd[i],&cmdargs,&num);
        execcmd(cmdargs,num);
    }
}


int main(){
    char * buffer = malloc(32);
    size_t bufsize = 32;
    int num;
    int cnum;
    char **cmdargs;
    char **cmd;
    
    while(1){
        printf("smash> ");
        fflush(stdout);
        getline(&buffer, &bufsize, stdin);
        lexer2(buffer, &cmd, &cnum);
        //printf("%d\n",cnum);
        if(cnum > 1){
            mulcmd(cmd,cnum);
        }else{
        lexer(cmd[0], &cmdargs, &num);
        execcmd(cmdargs,num);
    }
    }
}

