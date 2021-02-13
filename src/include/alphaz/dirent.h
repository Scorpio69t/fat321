#ifndef _ALPHAZ_DIRENT_H_
#define _ALPHAZ_DIRENT_H_

#include <alphaz/type.h>

#define DIR_BUF_SIZE    512

struct DIR {
    int fd;
    char buf[DIR_BUF_SIZE];
};

struct dirent {
    int type;
    loff_t size;
    int wdata, wtime;
    char name[256];
};

struct DIR * opendir(const char *);
struct DIR * closedir(struct DIR *);
struct dirent * readdir(struct DIR *);
int default_filldir(void *, const char *, int, loff_t, u64, int);

#endif
