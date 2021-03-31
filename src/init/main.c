#include "init.h"
#include <kernel/stdio.h>


extern void debug(char *);
int abc = 0;

static char buf[512];

int main(int argc, char *argv[])
{
    abc = 1;
    while(1) {
        int a = abc;
        int i = 0;
        while (a) {
            buf[i++] = '0' + (a % 10);
            a = a / 10;
        }
        buf[i] = 0;
        debug(buf);
        abc++;
    }
    return 0;
}
