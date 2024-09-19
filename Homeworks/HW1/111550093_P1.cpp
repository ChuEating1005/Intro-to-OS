/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER 100
#define EXIT 0
#define NORMAL 1
#define INPUT_FILE 2
#define OUTPUT_FILE 3
#define PIPE 4
#define ZOMBIE 5

using namespace std;

int status_check(char input[BUFFER]) {
    if(strcmp(input,"exit") == 0 || strcmp(input,"exit &") == 0){
        return EXIT;
    }
    else if(strchr(input, '<') != NULL){
        return INPUT_FILE;
    }
    else if(strchr(input, '>') != NULL){
        return OUTPUT_FILE;
    }
    else if(strchr(input, '|') != NULL){
        return PIPE;
    }
    else if(strchr(input, '&') != NULL){
        return ZOMBIE;
    }
    else{
        return NORMAL;
    }
} 

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void piping(char **lcmd, char **rcmd) {
    int fd[2];
    pid_t pid1, pid2;

    if (pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    
    pid1 = fork();
    if (pid1 == 0) {  // Child process 1 (executes left command, e.g., `ls`)
        close(fd[0]); // Close the read end of the pipe
        dup2(fd[1], STDOUT_FILENO);  // Redirect standard output to the write end of the pipe
        close(fd[1]); // Close the write end of the pipe
        execvp(lcmd[0], lcmd);  // Execute left command
        perror("execvp failed");
        exit(1);
    }
    
    pid2 = fork();
    if (pid2 == 0) {  // Child process 2 (executes right command, e.g., `more`)
        close(fd[1]);   // Close the write end of the pipe
        dup2(fd[0], STDIN_FILENO);  // Redirect standard input to the read end of the pipe
        close(fd[0]);   // Close the read end of the pipe
        execvp(rcmd[0], rcmd);  // Execute right command
        perror("execvp failed");
        exit(1);
    }

    // Parent process (wait for child processes)
    close(fd[0]);
    close(fd[1]); // Parent doesn't need the pipe, close it
    waitpid(pid1, NULL, 0);  // Wait for the left-side child process to finish
    waitpid(pid2, NULL, 0);  // Wait for the right-side child process to finish
}

void redirect_stdio(int status, char **lcmd, char **rcmd, char **file) {
    pid_t pid;
    pid = fork();
    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if (pid == 0) { /* child process */
        int fd;
        if (status == INPUT_FILE){
            // Redirect inupt
            fd = open(*file, O_RDONLY, S_IRUSR | S_IWUSR);
            dup2(fd, STDIN_FILENO);
        }
        else if (status == OUTPUT_FILE) {
            // Redirect output
            fd = open(*file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(fd, STDOUT_FILENO);
        }
        close(fd);
        execvp(lcmd[0], lcmd);
        exit(1);
    }
    else { /* parent process */
        waitpid(pid, NULL, 0);
    }
}

void execute_cmd(char **cmd, bool zombie) {
    pid_t  pid = fork();
    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if (pid == 0) { /* child process */
        execvp(cmd[0], cmd);
        exit(1);
    }
    else { /* parent process */
        if(zombie) {
            waitpid(-1, NULL, WNOHANG);
        }
        else {
            waitpid(pid, NULL, 0);
        }
    }
}

void split_command(char *cmd, char **lcmd, char **rcmd, char **file) {
    char *tokens[BUFFER];
    bool pipe_flag = false;
    int num_tok = 0;

    char *token = strtok(cmd, " ");
    while (token != NULL) {
        tokens[num_tok++] = token;
        token = strtok(NULL, " ");
    }

    int lidx = 0, ridx = 0;
    for (int j = 0; j < num_tok; j++) {
        // Check if the token is a pipe
        if (strcmp(tokens[j], "|") == 0) {
            pipe_flag = true;
            continue;
        }
        else if (strcmp(tokens[j], ">") == 0 || strcmp(tokens[j], "<") == 0) {
            *file = tokens[j+1];
            lcmd[lidx] = NULL;
            break;
        }

        if (!pipe_flag) {
            if (strcmp(tokens[j], "&") == 0) {
                lcmd[lidx] = NULL;
                break;
            }
            lcmd[lidx++] = tokens[j];
        } 
        else {
            rcmd[ridx++] = tokens[j];
        }
    }

    lcmd[lidx] = NULL;
    rcmd[ridx] = NULL;
}


int main() {
    char input[BUFFER];

    signal(SIGCHLD, sigchld_handler); // Handle zombie processes with SIGCHLD
    
    printf("> ");
    while(fgets(input, BUFFER, stdin) != NULL){        
        input[strlen(input) - 1] = '\0';
        int status = status_check(input);
        if (status == EXIT) break;

        char *lcmd[BUFFER], *rcmd[BUFFER], *file = NULL;
        split_command(input, lcmd, rcmd, &file);

        switch(status){
            case PIPE: {
                piping(lcmd, rcmd);
                break;
            }
            case INPUT_FILE:
            case OUTPUT_FILE: {
                redirect_stdio(status, lcmd, rcmd, &file);
                break;
            }
            case ZOMBIE: {
                execute_cmd(lcmd, true);
                break;
            }
            case NORMAL: {
                execute_cmd(lcmd, false);
                break;
            }
        }
        printf("> ");
    }
}