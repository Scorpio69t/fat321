#ifndef _SYS_FS_H_
#define _SYS_FS_H_

#include <dirent.h>
#include <sys/dentry.h>
#include <sys/stat.h>
#include <sys/types.h>

struct fs_ops {
    int (*fs_lookup)(const char *filename, ino_t pino, struct dentry *dentry);
    ssize_t (*fs_read)(ino_t ino, void *buf, off_t pos, size_t size);
    ssize_t (*fs_write)(ino_t ino, void *buf, off_t pos, size_t size);
    int (*fs_stat)(ino_t ino, struct stat *buf);
    ssize_t (*fs_getdents)(ino_t ino, struct dirent *dirp, off_t pos, size_t nbytes);
    int (*fs_init)(unsigned long, struct dentry *);
};

int run_fs(const char *fsname, const char *pmnt, struct fs_ops *ops);

#endif
