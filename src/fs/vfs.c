#include <alphaz/string.h>
#include <alphaz/type.h>
#include <alphaz/malloc.h>
#include <alphaz/kernel.h>
#include <alphaz/fs.h>
#include <alphaz/slab.h>
#include <alphaz/bugs.h>
#include <asm/irq.h>

struct super_block *root_sb = NULL;
struct dentry *root_dentry = NULL;

/**
 * register_filesystem - 注册文件系统
 * @fs: 要注册的文件系统指针
 */
int register_filesystem(struct file_system_type *fs)
{
    if (!fs)
        return -1;
    root_sb = fs->get_sb(fs, NULL);
    if (!root_sb)
        return -1;
    return 0;
}

/**
 * mount_fs - 挂载文件系统
 * @name: 要挂载的文件系统的名称
 * @sb:   要挂载的文件系统的超级块指针
 */
int mount_fs(const char *name, struct super_block *sb)
{
    return 0;
}


/**
 * 创建一个file对象并进行必要的初始化
 * @dentry: file对应的dentry结构体
 * @flags: 文件的标识
 * @mode: 文件的读取方式
 */
struct file * make_file(struct dentry *dentry, int flags, int mode)
{
    struct file *filp;

    filp = (struct file *)kmalloc(sizeof(struct file), 0);
    assert(filp != NULL);
    filp->f_dentry = dentry;
    filp->f_op = dentry->d_inode->i_fop;
    spin_init(&filp->f_lock);
    atomic_set(1, &filp->f_count);
    filp->f_flags = flags;
    filp->f_mode = mode;
    filp->f_pos = 0;

    return filp;
}

/*
 * 在缓存中寻找是否存在名为name的子目录
 */
static struct dentry * find_dcache(struct dentry *parent, const char *name, size_t len)
{
    struct dentry *child;
    size_t n;

    list_for_each_entry(child, &parent->d_subdirs, d_child) {
        n = strlen(child->d_name);
        if (n != len)
            continue;
        if (!strncmp(child->d_name, name, len))
            return child;
    }
    return NULL;
}

/**
 * make_dentry - 为parent创建一个子dentry并进行必要的初始化
 * @parent: 父目录项
 * @name: 要创建的dentry的文件名
 * @len: 文件名的长度
 */
struct dentry * make_dentry(struct dentry *parent, char *name, size_t len)
{
    struct dentry *entry;

    entry = (struct dentry *)kmalloc(sizeof(struct dentry), 0);
    assert(entry != NULL);
    atomic_set(1, &entry->d_count);
    entry->d_flags= 0;
    spin_init(&entry->d_lock);
    entry->d_inode = NULL;
    entry->d_parent = parent;
    memcpy(entry->d_name, name, len);
    entry->d_name[len] = 0;
    list_add_tail(&entry->d_child, &parent->d_subdirs);
    list_head_init(&entry->d_subdirs);
    entry->d_op = parent->d_op;
    entry->d_sb = parent->d_sb;
    entry->d_private = parent->d_private;
    return entry;
}

struct dentry * path_walk(const char *path, int flags)
{
    char *name, *p;
    int len;
    struct dentry *parent = root_sb->s_root;
    struct dentry *child;

    if (!strcmp(path, "/"))
        return root_sb->s_root;

    p = (char *)path;
    while (*p == '/')
        p++;
    if (!*p)
        return NULL;

    while (1) {
        name = p;
        len = 0;
        while (*p && *p != '/') {
            p++;
            len++;
        }

        if (!(child = find_dcache(parent, name, len)))
            child = make_dentry(parent, name, len);
        if (parent->d_inode->i_op->lookup(parent->d_inode, child) == NULL) {
            kfree(child);
            return NULL;
        }

        if (!*p) {
            break;
        }

        while (*p == '/')
            p++;
        if (!*p)
            break;

        parent = child;
        child = NULL;
    }
    return child;
}
