#ifndef _SYS_FS_H_
#define _SYS_FS_H_

#include <sys/dentry.h>
#include <sys/stat.h>
#include <sys/types.h>

struct fs_ops {
    int (*fs_lookup)(const char *filename, ino_t pino, struct dentry *dentry);
    ssize_t (*fs_read)(ino_t ino, void *buf, loff_t pos, size_t size);
    ssize_t (*fs_write)(ino_t ino, void *buf, loff_t pos, size_t size);
    int (*fs_stat)(ino_t ino, struct stat *buf);
    int (*fs_init)(unsigned long, struct dentry *);
};

int run_fs(const char *fsname, const char *pmnt, struct fs_ops *ops);

#endif
