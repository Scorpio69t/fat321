#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char *usage = "Usage: ls [DIRECTORY]...";

int ls(const char *path)
{
    DIR *dp;
    struct dirent *dirp;

    if (!(dp = opendir(path)))
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

int main(int argc, char *argv[])
{
    int i;

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        printf("%s\n", usage);
        return 0;
    }
    if (argc == 1)
        return ls(".");

    for (i = 1; i < argc; i++) {
        if (argc > 2)
            printf("%s:\n", argv[i]);
        if (ls(argv[i]) < 0) {
            printf("ls: no such directory '%s'\n", argv[i]);
            return -1;
        }
    }
    return 0;
}