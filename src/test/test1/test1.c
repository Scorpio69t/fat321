#include <sys/ipc.h>
#include <sys/syscall.h>
#include <stdio.h>


int main()
{
    message msg;

    for (int i = 0; i < 10; i++) {
        sys_send(2, &msg);
    }
    for (int i = 0; i < 10; i++) {
        sys_recv(2, &msg);
    }
    printf("test1 done\n");
    while(1){}
}
