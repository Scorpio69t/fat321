#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "init.h"

int main(int argc, char *argv[])
{
    int pid;

    if ((pid = fork()) == 0) {
        execve("/bin/sh", NULL, NULL);
    }

    if (pid == -1)
        goto failed;

    while (1) {
        wait(NULL);
    }

failed:
    perror("init failed\n");
    while (1) nop();
}
