#include <boot/unistd.h>
#include <kernel/bugs.h>
#include <kernel/dirent.h>
#include <kernel/fcntl.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/unistd.h>

unsigned long sys_get_ticks(void)
{
    return ticks;
}

ssize_t sys_write(int fd, const void *buf, size_t nbytes)
{
    // struct file *filp;
    // int          ret = -1;

    // if (fd < 0 || fd >= PROC_MAX_FILE)
    //     return -1;
    // if (nbytes < 0)
    //     return -1;
    // // filp = current->files->files[fd];
    // if (filp && filp->f_op && filp->f_op->write)
    //     ret = filp->f_op->write(filp, buf, nbytes, filp->f_pos);
    // if (ret != -1)
    //     filp->f_pos += ret;
    // return ret;
}

ssize_t sys_read(int fd, void *buf, size_t nbytes)
{
    // struct file *filp;
    // int          ret = -1;

    // if (fd < 0 || fd >= PROC_MAX_FILE)
    //     return -1;
    // if (nbytes < 0)
    //     return -1;
    // // filp = current->files->files[fd];
    // if (filp && filp->f_op && filp->f_op->read)
    //     ret = filp->f_op->read(filp, buf, nbytes, filp->f_pos);
    // if (ret != -1)
    //     filp->f_pos += ret;
    // return ret;
}

int sys_open(const char *path, int oflag)
{
    // struct dentry *de = NULL;
    // struct file *  filp = NULL;
    // int            fd;

    // if (atomic_read(&current->files->count) >= PROC_MAX_FILE)
    //     goto open_faild;

    //     de = path_walk(path, 0);
    //     if (!de)
    //         goto open_faild;

    //     if ((oflag & O_DIRECTORY) && !(de->d_inode->i_flags & FS_ATTR_DIR))
    //         return -1;

    //     filp = make_file(de, 0, oflag);
    //     if (!filp)
    //         goto open_faild;

    //     for (fd = 0; fd < PROC_MAX_FILE; fd++) {
    //         // if (!current->files->files[fd])
    //             break;
    //     }

    //     // atomic_inc(&current->files->count);
    //     // current->files->files[fd] = filp;
    //     return fd;

    // open_faild:
    //     if (de)
    //         kfree(de);
    //     if (filp)
    //         kfree(filp);
    return -1;
}

int sys_close(int fd)
{
    // struct file *filp;

    // if (fd < 0 || fd >= PROC_MAX_FILE)
    //     return -1;
    // // filp = current->files->files[fd];
    // // current->files->files[fd] = NULL;
    // // atomic_dec(&current->files->count);
    // atomic_dec(&filp->f_count);

    // if (!atomic_read(&filp->f_count))
    //     kfree(filp);
    // return 0;
}

int sys_chdir(const char *path)
{
    struct dentry *de;

    // if (!strcmp(path, ".."))
    // //     de = current->cwd->d_parent;
    // else if (!strcmp(path, "."))
    //     return 0;
    // else
    //     de = path_walk(path, 0);

    // if (de == NULL)
    //     return -1;
    // if (!(de->d_inode->i_flags & FS_ATTR_DIR))
    //     return -1;
    // current->cwd = de;
    return 0;
}

int sys_getcwd(char *buf, size_t n)
{
    // struct dentry *cur;
    // struct dentry *tmp[16];
    // int            i = 0, ret = 0, len, tn;
    // char *         p = buf;
    // // cur = current->cwd;
    // if (cur == NULL)
    //     return -1;

    // while (cur) {
    //     tmp[i++] = cur;
    //     if (cur != cur->d_parent) /* 根目录的父目录可能是自己也可能为空 */
    //         cur = cur->d_parent;
    //     else
    //         cur = NULL;
    // }

    // tn = n - 1;
    // for (i = i - 1; i >= 0 && tn; i--) {
    //     len = strlen(tmp[i]->d_name);
    //     strncpy(p, tmp[i]->d_name, tn);
    //     p += len < tn ? len : tn;
    //     ret += len < tn ? len : tn;
    //     n -= len < tn ? len : tn;

    //     if (tn && *(p - 1) != '/') {
    //         *p++ = '/';
    //         ret++;
    //         tn--;
    //     }
    // }
    // *p = 0;
    // return ret;
}

int sys_getdents(int fd, void *dirent, int count)
{
    // struct file *filp;
    // int          ret = -1;

    // if (fd < 0 || fd >= PROC_MAX_FILE)
    //     return -1;
    // if (count < 0)
    //     return -1;

    // filp = current->files->files[fd];
    // if (filp && filp->f_op && filp->f_op->readdir)
    //     ret = filp->f_op->readdir(filp, dirent, default_filldir);
    // return ret;
}

unsigned long sys_getpid(void)
{
    return current->pid;
}

long sys_reboot(void)
{
    // __sys_reboot();
    return 0;
}

long do_brk(unsigned long addr)
{
    proc_t *      proc = current;
    unsigned long new_brk, brk;
    int           nr_pages, i;

    new_brk = PAGE_UPPER_ALIGN(addr);
    nr_pages = (new_brk - proc->mm.brk) / PAGE_SIZE;

    for (brk = proc->mm.brk, i = 0; i < nr_pages; i++, brk += PAGE_SIZE) {
        if (map_page(proc, brk) == 0) {
            printk("pid: %d do_brk faild\n", current->pid);
            return -1;
        }
    }
    proc->mm.brk = new_brk;
    return 0;
}
