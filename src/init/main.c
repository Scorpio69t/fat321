#include <stdio.h>
#include <sys/ipc.h>
#include <sys/syscall.h>

#include "init.h"

int abc = 0;

static char buf[512];

int main(int argc, char *argv[])
{
    message msg = {.type = MSG_DISK,
                   .m_disk = {
                       .type = DISK_READ,
                       .nsect = 1,
                       .sector = 1,
                       .buf = buf,
                   }};

    message a;

    sys_send(IPC_DISK, &msg);
    printf("init send done\n");
    sys_recv(IPC_DISK, &a);
    printf("init recv done %d\n", a.type);

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            printf("%c ", buf[i*16 + j]);
        }
        printf("\n");
    }

    while (1) {}
    return 0;
}
