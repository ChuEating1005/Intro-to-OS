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

#define MAX_TOKENS 100

using namespace std;


void split_command(char *cmd, char **lcmd, char **rcmd, bool &pipe, bool &wait_child) {
    char *tokens[MAX_TOKENS];
    int num_tok = 0;

    char *token = strtok(cmd, " ");
    while (token != NULL) {
        tokens[num_tok++] = token;
        token = strtok(NULL, " ");
    }

    char *last1 = NULL, *last2 = NULL;
    for (int j = 0; j < num_tok; j++) {
        // Check if the token is a pipe
        if (strcmp(tokens[j], "|") == 0) {
            pipe = true;
            continue;
        }
        else if (strcmp(tokens[j], "&") == 0) {
            wait_child = true;
            break;
        }

        if (!pipe) {
            *lcmd++ = tokens[j];
            last1 = tokens[j];
        } 
        else {
            *rcmd++ = tokens[j];
            last2 = tokens[j];
        }

        if (strcmp(last1, "&") == 0) {
            wait_child = false;
            *lcmd--;
            break;
        }
        *lcmd = NULL;

        if (pipe) {
            if (strcmp(last2, "&") == 0) {
                wait_child = false;
                *rcmd--;
                break;
            }
            *rcmd = NULL;
        }
    }
}


int main() {

    pid_t  pid;

    printf(">");
    char input[100];

    while(fgets(input, 100, stdin) != NULL){
        char *lcmd[MAX_TOKENS], *rcmd[MAX_TOKENS];
        bool pipe = false, wait_child = true;
        split_command(input, lcmd, rcmd, pipe, wait_child);
        
        pid = fork();

        if (pid < 0) { /* error occurred */
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if (pid == 0) { /* child process */
            execvp(lcmd[0], lcmd);
        }
        else { /* parent process */
            /* parent will wait for the child to complete */
            wait (NULL);
        }
    }
}