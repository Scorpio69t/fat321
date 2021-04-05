#include <stdio.h>
#include <sys/ipc.h>
#include <sys/syscall.h>

int main()
{
    message msg;

    for (int i = 0; i < 10; i++) {
        _send(2, &msg);
    }
    for (int i = 0; i < 10; i++) {
        _recv(2, &msg);
    }
    printf("test1 done\n");
    while (1) {
    }
}
