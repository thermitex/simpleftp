#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {

    char cmd[1000];
    char *args[20];
    int n = 0;
    char *ptr;
    char *status = "(offline)";

    while (1) {
        printf("%s ", status);
        fflush(stdout);
        if (!fgets(cmd, 1000, stdin))
            break;
        ptr = index(cmd, 0);
        if (ptr[-1] != '\n') {
            fprintf(stderr, "simpleftp.cli: command too long\n");
            break;
        }
        ptr = cmd; // parse the command
        for (n = 0; (args[n] = strtok(ptr, " \n")); n++, ptr = NULL);
        if (n <= 0) // no input
            continue;
        if (!strcmp(args[0], "exit")) // exit
            exit(0);
    }
    
}