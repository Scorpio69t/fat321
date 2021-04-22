#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // int pid, status;
    // if ((pid = fork()) == 0) {
    //     if (execve("/usr/bin/hello", NULL, NULL)) {
    //         printf("exec error\n");
    //     }
    // }
    // printf("shell child pid: %d\n", pid);
    // pid = wait(&status);
    // printf("shell wait %d %d\n", pid, status);
    char buf[512];
    printf("shell pwd %s\n", getcwd(buf, 512));
    if (chdir("../home/") != 0) {
        printf("chdir error\n");
    }
    printf("shell pwd %s\n", getcwd(buf, 512));
    return 0;
}
