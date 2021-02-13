#ifndef _ALPHAZ_VFS_H_
#define _ALPHAZ_VFS_H_

#include <alphaz/blkdev.h>
#include <alphaz/list.h>
#include <alphaz/dirent.h>
#include <alphaz/spinlock.h>
#include <asm/atomic.h>

/* lseek调用选项 */
#define  SEEK_SET       (1 << 0)
#define  SEEK_CUR       (1 << 1)
#define  SEEK_END       (1 << 2)

/* 文件属性 */
#define FS_ATTR_FILE    (1 << 0)
#define FS_ATTR_DIR     (1 << 1)
#define FS_ATTR_DEVICE  (1 << 2)

extern struct super_block *root_sb;
extern struct dentry *root_entry;

struct file_system_type {
	const char  *name;         /* 文件系统名称 */
	int         fs_flags;
	struct super_block *(*get_sb)(struct file_system_type *, void *);  /* 从磁盘中读取超级块 */
    void (*put_sb) (struct super_block *);  /* 终止访问超级块并释放 */
	struct file_system_type *next;
};

extern struct file_system_type file_system;

struct super_block {
    unsigned long           s_blocksize;    /* 以字节位单位的块大小 */
    struct file_system_type *s_type;        /* 文件系统类型 */
    struct super_operations *s_op;
    struct dentry           *s_root;        /* 根目录的挂载点   */
    struct list_head        s_inodes;       /* 所有inode的链表  */
    struct list_head        s_dirty;        /* 脏inode链表 */
    void                    *s_fs_info;     /* 文件系统的私有信息 */
};

struct super_operations {
	struct inode *(*alloc_inode)(struct super_block *);     /* 创建并初始化一个inode */
    void (*destory_inode)(struct inode *);                  /* 释放一个给定节点 */
    void (*write_inode) (struct inode *);                   /* 将给定的inode写入磁盘 */
    void (*delete_inode) (struct inode *);                  /* 从磁盘上删除给定节点 */
    void (*write_super) (struct super_block *);             /* 更新磁盘上的超级块 */
    void (*put_super) (struct super_block *);               /* 卸载文件系统时由vfs调用，释放超级块 */
};

struct inode {
    struct list_head        i_sb_list;          /* 超级块链表，挂载到super_block的s_inodes或s_dirty */
    struct list_head        i_denty;            /* 目录项链表 */
    unsigned long           i_ino;              /* inode号 */
    atomic_t                i_count;            /* 引用计数 */
    struct inode_operations *i_op;
    struct file_operations  *i_fop;             /* 文件相关操作 */
    spinlock_t              i_lock;
    unsigned long           i_size;             /* 以字节为单位的文件大小 */
    struct super_block      *i_sb;              /* 相关的超级块 */
    unsigned long           i_state;            /* 状态标志 */
    unsigned int            i_flags;            /* inode属性 */
    void                    *i_private;         /* 文件系统私有信息 */
};

struct inode_operations {
	int (*create)(struct inode *, struct dentry *, int mode);            /* 为dentry创建一个新的索引节点 */
	struct dentry *(*lookup)(struct inode *dir, struct dentry *);        /* 在特定的目录中寻找节点, 该节点对应dentry中的文件名 */
	int (*mkdir)(struct inode *dir, struct dentry *, int mode);          /* 在特定的目录中创建一个新节点 */
	int (*rmdir)(struct inode *, struct dentry *);                       /* 删除dir目录中由dentry代表的文件 */
	int (*rename)(struct inode *old_dir, struct dentry *, struct inode *new_dir,    /* 移动文件 */
				  struct dentry *);
	int (*getattr)(struct dentry *, unsigned long);
	int (*setattr)(struct dentry *, unsigned long);
};

struct dentry {
    atomic_t                d_count;            /* 使用计数 */
    unsigned int            d_flags;
    spinlock_t              d_lock;
    struct inode            *d_inode;           /* 与该目录项相关联的索引节点 */
    struct dentry           *d_parent;          /* 父目录项 */
    char                    d_name[128];        /* 目录项名称 */
    struct list_head        d_child;            /* 挂载到父目录项的d_subdirs */
    struct list_head        d_subdirs;          /* 子目录链表 */
    struct dentry_operations *d_op;
    struct super_block      *d_sb;              /* 文件的超级块 */
    void                    *d_private;         /* 文件系统私有数据 */
};

struct dentry_operations {
	int (*d_compare)(struct dentry *, char *, char *);    /* 文件名比较 */
	int (*d_release)(struct dentry *);                    /* 目录项释放时调用 */
	int (*d_iput)(struct dentry *, struct inode *);       /* 释放索引节点 */
};

struct file {
	struct dentry          *f_dentry;
    struct file_operations *f_op;
    spinlock_t              f_lock;
    atomic_t                f_count;    /* 引用计数 */
    unsigned int            f_flags;    /* 打开文件时指定的标志 */
    unsigned int            f_mode;     /* 文件访问模式 */
    loff_t                  f_pos;      /* 当前文件位移量 */
};

typedef int (*filldir_t) (void *, const char *, int, loff_t, u64, int);

struct file_operations {
	loff_t (*lseek) (struct file *, loff_t, int);        /* 改变文件的偏移位置 */
    ssize_t (*read) (struct file *, char *, size_t, loff_t);
    ssize_t (*write) (struct file *, const char *, size_t, loff_t);
    int (*open) (struct inode *, struct file *);    /* 创建一个新的对象文件，并将其与相应的索引节点关联起来 */
    int (*release) (struct inode *, struct file *); /* 引用计数为0时，由vfs调用 */
    int (*readdir) (struct file *, void *, filldir_t);
};

int register_filesystem(struct file_system_type *);
int mount_fs(const char *, struct super_block *);

struct file * make_file(struct dentry *, int, int);
struct dentry * make_dentry(struct dentry *, char *, size_t);

struct dentry * path_walk(const char *path, int flags);

#endif
