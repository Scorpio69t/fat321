#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int getline(char *buf)
{
    int len;
    char c;

    len = 0;
    do {
        if (read(0, &c, 1) != 1)
            break;
        switch (c) {
        case '\t':
        case '\b':
        default:
            buf[len++] = c;
            write(1, &c, 1);
        }
    } while (c != '\n');
    buf[len] = 0;
    return len;
}

static char buffer[512];

int main()
{
    int fd, n;

    if ((fd = open("/b", O_APPEND)) < 0) {
        perror("open file failed\n");
        exit(-1);
    }
    while ((n = getline(buffer)) != 0 && buffer[0] != '\n') {
        write(fd, buffer, n);
    }
    close(fd);
    return 0;
}