#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char *usage = "Usage: cat [FILE]...";

int main(int argc, char *argv[])
{
    struct stat sbuf;
    int i, fd, n;
    char buf[512];

    if (argc < 2 || (argc == 2 && !strcmp(argv[1], "--help"))) {
        printf("%s\n", usage);
        return -1;
    }

    for (i = 1; i < argc; i++) {
        if (stat(argv[i], &sbuf) < 0) {
            printf("cat: stat %s failed\n", argv[i]);
            return -1;
        }
        if (!(sbuf.st_mode & S_IFREG)) {
            printf("cat: %s is not a file\n", argv[i]);
            return -1;
        }
        if ((fd = open(argv[i], O_RDONLY)) < 0) {
            printf("cat: open file %s failed\n", argv[i]);
        }
        while ((n = read(fd, buf, 512)) > 0) {
            write(1, buf, n);
        }
        close(fd);
    }
    return 0;
}