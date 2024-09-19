#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 100

void split_command(char *cmd) {
    char *tokens[MAX_TOKENS];
    int i = 0;

    // Use strtok to split by spaces
    char *token = strtok(cmd, " ");
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }

    // Print the result
    for (int j = 0; j < i; j++) {
        printf("Token %d: %s\n", j, tokens[j]);
    }
}

int main() {
    char command[] = "ls -la /home/user";
    split_command(command);
    return 0;
}