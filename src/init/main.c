#include <stdio.h>
#include <syscall.h>

#include "init.h"

int abc = 0;

static char buf[512];

int main(int argc, char *argv[])
{
    abc = 1;
    while (1) {
        sprintf(buf, "init %d\n", abc++);
        debug(buf);
    }
    return 0;
}
