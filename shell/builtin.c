#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>

#include "shell.h"

int cls(int argc, char *argv[])
{
    char c = 0x0c;
    write(1, &c, 1);
    return 0;
}

int pwd(int argc, char *argv[])
{
    int   retval;
    char *buf;

    if ((buf = (char *)malloc(512)) == 0) {
        panic("buffer alloc failed\n");
        return 1;
    }
    if ((buf = getcwd(buf, 512))) {
        printf("%s\n", buf);
        retval = 0;
    } else {
        panic("pwd get an error\n");
        retval = 1;
    }
    free(buf);
    return retval;
}
