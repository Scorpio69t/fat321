#include <sys/ipc.h>
#include <sys/syscall.h>
#include <stdio.h>


int main()
{
    message msg;

    for (int i = 0; i < 10; i++) {
        sys_recv(1, &msg);
    }
    for (int i = 0; i < 10; i++) {
        sys_send(1, &msg);
    }
    printf("test2 done\n");
    while(1){}
}
