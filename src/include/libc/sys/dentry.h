#ifndef _SYS_DENTRY_H_
#define _SYS_DENTRY_H_

#include <sys/list.h>
#include <sys/types.h>

#define DE_NORMAL 1
#define DE_DEV    2

struct dentry {
    pid_t f_fs_pid;              /* 文件系统进程 */
    unsigned int f_flags;        /* entry标识 */
    unsigned long f_ino;         /* inode号 */
    int f_count;                 /* 引用计数 */
    size_t f_size;               /* 文件大小 */
    mode_t f_mode;               /* 文件属性 */
    char f_name[128];            /* 文件名称 */
    struct dentry* f_parent;     /* 父entry */
    struct list_head f_list;     /* 挂载到父entry的f_children */
    struct list_head f_children; /* 子entry */
};

#endif
