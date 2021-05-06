#ifndef _VFS_H_
#define _VFS_H_

#include <sys/list.h>
#include <sys/types.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

struct file {
    mode_t         f_mode; /* 文件打开属性 */
    struct dentry *f_dentry;
    loff_t         f_pos;
};

struct vmount {
    pid_t            m_fs_pid;
    struct dentry *  m_entry;
    struct list_head list;
};

extern struct list_head mount_head;

#define NR_FILES 64
struct proc_file {
    pid_t            pid;  /* 进程pid */
    struct list_head list; /* 用于__file_map中处理hash冲突 */
    struct file *    filp[NR_FILES];
    struct dentry *  cwd;
};

struct dev_file {
    char   name[128];
    pid_t  driver_pid; /* 驱动pid */
    mode_t mode;
};

#endif
