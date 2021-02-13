#include <alphaz/dirent.h>
#include <alphaz/fs.h>
#include <alphaz/type.h>
#include <alphaz/stdio.h>
#include <alphaz/unistd.h>
#include <alphaz/malloc.h>
#include <alphaz/bugs.h>
#include <alphaz/stdio.h>
#include <alphaz/string.h>
#include <alphaz/fcntl.h>

struct DIR * opendir(const char *path)
{
    int fd;
    struct DIR *dir;

    fd = open(path, O_DIRECTORY);
    if (fd == -1)
        return NULL;

    dir = (struct DIR *)malloc(sizeof(struct DIR));
    if (!dir)
        return NULL;
    memset(dir, 0, sizeof(struct DIR));
    dir->fd = fd;
    return dir;
}

struct DIR * closedir(struct DIR *dir)
{
    close(dir->fd);
    free(dir);
    return 0;
}

struct dirent * readdir(struct DIR *dir)
{
    int len;

    len = getdents(dir->fd, (struct dirent *)dir->buf, DIR_BUF_SIZE);
    if (len <= 0)
        return NULL;
    return (struct dirent *)dir->buf;
}

int default_filldir(void *dirent, const char *name, int len, loff_t size, u64 time, int type)
{
    struct dirent *d;

    d = (struct dirent *)dirent;
    d->size = size;
    d->type = type;
    d->wdata = (time >> 32) & 0xffffffff;
    d->wtime = time &0xffffffff;
    strncpy(d->name, name, len);
    return 1;
}
