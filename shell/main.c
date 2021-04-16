#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("hello world\n");
    printf("shell %d\n", getpid());
    while (1) nop();
}
