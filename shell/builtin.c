#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shell.h"

int cls(int argc, char *argv[])
{
    char c = 0x0c;
    write(1, &c, 1);
    return 0;
}

int pwd(int argc, char *argv[])
{
    int retval;
    char *buf;

    if ((buf = (char *)malloc(512)) == 0) {
        panic("buffer alloc failed\n");
        return 1;
    }
    if ((buf = getcwd(buf, 512))) {
        printf("%s\n", buf);
        retval = 0;
    } else {
        panic("pwd get an error\n");
        retval = 1;
    }
    free(buf);
    return retval;
}

// TODO: more feature
int ls(int argc, char *argv[])
{
    DIR *dp;
    struct dirent *dirp;

    if (!(dp = opendir(".")))
        return -1;
    while ((dirp = readdir(dp))) {
        if (!strcmp(".", dirp->d_name) || !strcmp("..", dirp->d_name))
            continue;
        printf("%s ", dirp->d_name);
    }
    printf("\n");
    closedir(dp);
    return 0;
}

int cd(int argc, char *argv[])
{
    struct stat sbuf;

    if (argc != 2) {
        perror("Usage: cd <dirname>\n");
        return -1;
    }
    if (stat(argv[1], &sbuf) != 0 || !(sbuf.st_mode & S_IFDIR)) {
        printf("No such directory: %s\n", argv[1]);
        return -1;
    }
    return chdir(argv[1]);
}
