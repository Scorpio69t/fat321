#ifndef _VFS_H_
#define _VFS_H_

#include <sys/list.h>
#include <sys/types.h>

struct fentry {
    pid_t            f_fs_pid;    /* 文件系统进程 */
    unsigned long    f_ino;       /* inode号 */
    int              f_count;     /* 引用计数 */
    size_t           f_size;      /* 文件大小 */
    mode_t           f_mode;      /* 文件属性 */
    loff_t           f_pread;     /* 读取位置 */
    loff_t           f_pwrite;    /* 写入位置 */
    char             f_name[128]; /* 文件名称 */
    struct fentry *  f_parent;    /* 父entry */
    struct list_head f_list;      /* 挂载到父entry的f_children */
    struct list_head f_children;  /* 子entry */
};

struct file {
    mode_t         f_mode; /* 文件打开属性 */
    struct ventry *f_entry;
    loff_t         f_pos;
};

struct vmount {
    pid_t            m_fs_pid;
    struct fentry *  m_entry;
    struct list_head list;
};

extern struct list_head mount_head;

#define NR_FILES 64
typedef struct proc_file {
    pid_t            pid; /* 进程pid */
    struct list_head list;
    struct file *    filp[NR_FILES];
} file_t;

#endif
