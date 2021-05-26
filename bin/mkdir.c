#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char *usage = "Usage: mkdir <dirname>";

int main(int argc, char *argv[])
{
    int i, len;
    struct stat sbuf;

    if (argc != 2 || !strcmp(argv[1], "--help")) {
        printf("%s\n", usage);
        return 0;
    }
    len = strlen(argv[1]);
    for (i = 0; i < len; i++) {
        if (argv[1][i] == '/') {
            perror("mkdir: Not support multi-level directory yet.\n");
            return -1;
        }
    }
    if (!stat(argv[1], &sbuf)) {
        perror("mkdir: Directory already exists\n");
        return -1;
    }
    if (mkdir(argv[1], S_IFDIR | 0x1fd) < 0)
        return -1;
    return 0;
}