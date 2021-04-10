#ifndef _VFS_H_
#define _VFS_H_

#include <sys/list.h>
#include <sys/types.h>

struct ventry {
    pid_t            v_fs_pid;    /* 文件系统进程 */
    unsigned long    v_ino;       /* inode号 */
    int              v_count;     /* 引用计数 */
    size_t           v_size;      /* 文件大小 */
    mode_t           v_mode;      /* 文件属性 */
    loff_t           v_pread;     /* 读取位置 */
    loff_t           v_pwrite;    /* 写入位置 */
    char             v_name[128]; /* 文件名称 */
    struct ventry *  v_parent;    /* 父entry */
    struct list_head v_list;      /* 挂载到父entry的v_children */
    struct list_head v_children;
};

struct file {
    mode_t         f_mode; /* 文件打开属性 */
    struct ventry *f_entry;
    loff_t         f_pos;
};

struct vmount {
    pid_t          m_fs_pid;
    char           m_path[128];
    struct ventry *m_entry;
};

#define NR_FILES 64
typedef struct proc_file {
    pid_t            pid; /* 进程pid */
    struct list_head list;
    struct file *    filp[NR_FILES];
} file_t;

#endif
