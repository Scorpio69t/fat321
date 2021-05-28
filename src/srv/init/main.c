#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "init.h"

static char *shargv[] = {
    "/bin/sh",
    NULL,
};
static char *shenvp[] = {
    "PATH=/bin:/usr/bin",
    NULL,
};

int main(int argc, char *argv[])
{
    int pid;

    if ((pid = fork()) == 0) {
        execve("/bin/sh", shargv, shenvp);
        panic("execve /bin/sh failed\n");
        exit(-1);
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
