#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <sys/types.h>

struct stat {
    mode_t st_mode;
    unsigned int st_ino;
    off_t st_size;
    unsigned long st_atime;
    unsigned long st_mtime;
    unsigned long st_ctime;
};

int stat(const char *pathname, struct stat *buf);

#endif
