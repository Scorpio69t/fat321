#ifndef _SYS_FS_H_
#define _SYS_FS_H_

#include <sys/types.h>

struct fs_entry {
    unsigned long inode;
    loff_t        pread;
    loff_t        pwrite;
    mode_t        mode;
    size_t        fsize;
};

struct fs_ops {
    int (*fs_lookup)(const char *filename, struct fs_entry *p_entry, struct fs_entry *entry);
    ssize_t (*fs_read)(const struct fs_entry *entry, void *buf, loff_t pos, size_t size);
    ssize_t (*fs_write)(const struct fs_entry *entry, void *buf, loff_t pos, size_t size);
    int (*fs_init)(unsigned long, struct fs_entry *);
};

int run_fs(struct fs_ops *ops);

#endif
