#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

DIR *opendir(const char *pathname)
{
    DIR *dp;
    int fd;

    if ((fd = open(pathname, O_RDONLY)) < 0)
        return NULL;
    if (!(dp = (DIR *)malloc(sizeof(DIR))))
        return NULL;
    if (!(dp->__data = malloc(DIR_DATA_SIZE))) {
        free(dp);
        return NULL;
    }
    dp->__fd = fd;
    dp->__size = DIR_DATA_SIZE;
    dp->__entry = 0;
    return dp;
}

struct dirent *readdir(DIR *dp)
{
    struct dirent *dirp;

    if (!dp->__entry || ((struct dirent *)dp->__entry)->d_ino == 0) {
        memset(dp->__data, 0x00, DIR_DATA_SIZE);
        if (getdents(dp->__fd, (struct dirent *)dp->__data, dp->__size - 16) <= 0)
            return NULL;
        dp->__entry = dp->__data;
    }
    dirp = (struct dirent *)dp->__entry;
    dp->__entry = dp->__entry + dirp->d_reclen;
    return dirp;
}

int closedir(DIR *dp)
{
    free(dp->__data);
    free(dp);
    return 0;
}
