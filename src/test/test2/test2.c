#include <stdio.h>
#include <sys/ipc.h>
#include <sys/syscall.h>

int main()
{
    message msg;

    for (int i = 0; i < 10; i++) {
        _recv(1, &msg);
    }
    for (int i = 0; i < 10; i++) {
        _send(1, &msg);
    }
    printf("test2 done\n");
    while (1) {
    }
}
