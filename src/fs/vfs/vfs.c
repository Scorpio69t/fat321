#include "vfs.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/list.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "blkdev.h"

struct list_head      mount_head;
static struct fentry *root_entry;
static struct fentry *stdin, *stdout, *stderr;

#define __FILE_MAP_SIZE 1024
static struct list_head __file_map[__FILE_MAP_SIZE];

static struct proc_file *map_proc_file(pid_t pid)
{
    struct list_head *head;
    struct proc_file *pos;

    head = &__file_map[pid % __FILE_MAP_SIZE];
    list_for_each_entry(pos, head, list)
    {
        if (pos->pid == pid)
            return pos;
    }
    return NULL;
}

static int unmap_proc_file(pid_t pid)
{
    struct proc_file *file;

    if ((file = map_proc_file(pid)) == NULL)
        return 0;

    list_del(&file->list);
    return 0;
}

static int append_proc_file(struct proc_file *fsp)
{
    struct list_head *head;
    struct proc_file *pos;

    assert(fsp->pid != 0);

    head = &__file_map[fsp->pid % __FILE_MAP_SIZE];
    list_for_each_entry(pos, head, list)
    {
        if (pos->pid == fsp->pid)
            return -1;
    }
    list_add_tail(&fsp->list, head);
    return 0;
}

static struct file *make_filp(mode_t mode, struct fentry *entry)
{
    struct file *filp;

    filp = (struct file *)malloc(sizeof(struct file));
    assert(filp != NULL);
    filp->f_mode = mode;
    filp->f_entry = entry;
    filp->f_pos = 0;
    entry->f_count++;
    return filp;
}

static void free_filp(struct file *filp)
{
    filp->f_entry->f_count--;
    free(filp);
}

/* 为进程初始化 */
static struct proc_file *init_proc_file(pid_t pid)
{
    struct proc_file *proc_file;

    proc_file = (struct proc_file *)malloc(sizeof(struct proc_file));
    assert(proc_file != NULL);

    memset(proc_file, 0x00, sizeof(struct proc_file));
    proc_file->pid = pid;
    proc_file->cwd = root_entry;
    root_entry->f_count++;

    proc_file->filp[STDIN_FILENO] = make_filp(stdin->f_mode, stdin);
    proc_file->filp[STDOUT_FILENO] = make_filp(stdout->f_mode, stdout);
    proc_file->filp[STDERR_FILENO] = make_filp(stderr->f_mode, stderr);

    return proc_file;
}

static struct file *getfilp(pid_t pid, int fd)
{
    struct proc_file *proc_file;

    proc_file = map_proc_file(pid);
    if (proc_file == NULL) {
        proc_file = init_proc_file(pid);
        append_proc_file(proc_file);
    }

    if (!(fd >= 0 && fd < NR_FILES))
        return NULL;
    return proc_file->filp[fd];
}

int getfilename(char *sptr, char *buffer)
{
    int len = 0;

    while (*sptr != 0 && *sptr != '/') buffer[len++] = *sptr++;
    buffer[len] = 0;
    return len;
}

static struct fentry *vfs_lookup_in_fs(struct fentry *parent, const char *filename)
{
    struct fentry *child;
    message        mess;

    if (_kmap((void **)&filename, NULL, NULL) != 0) {
        debug("vfs_lookup_in_fs kmap failed\n");
        return NULL;
    }
    mess.type = MSG_FSLOOKUP;
    mess.m_fslookup.filename = (char *)filename;
    mess.m_fslookup.p_inode = parent->f_ino;
    mess.m_fslookup.p_pread = parent->f_pread;
    if (_sendrecv(parent->f_fs_pid, &mess) != 0) {
        debug("vfs_lookup_in_fs failed\n");
        return NULL;
    }
    if (mess.retval != 0) {
        debug("vfs_lookup_in_fs unfound %s in %s\n", filename, parent->f_name);
        return NULL;
    }

    child = (struct fentry *)malloc(sizeof(struct fentry));
    assert(child != NULL);
    child->f_fs_pid = parent->f_fs_pid;
    child->f_flags = FE_NORMAL;
    child->f_ino = mess.m_fslookup.inode;
    child->f_count = 1;
    child->f_size = mess.m_fslookup.fsize;
    child->f_mode = mess.m_fslookup.mode;
    child->f_pread = mess.m_fslookup.pread;
    child->f_pwrite = mess.m_fslookup.pwrite;
    strcpy(child->f_name, filename);
    child->f_parent = parent;
    list_add(&child->f_list, &parent->f_children);
    list_head_init(&child->f_children);
    parent->f_count++;

    return child;
}

static struct fentry *vfs_lookup(struct fentry *cwd, const char *path)
{
    char *sptr;
    int   len;
    char  filename[256];

    struct fentry *parent, *child, *pos;

    assert(cwd != NULL);
    if (!strncmp("/", path, 1)) {
        parent = root_entry;
        sptr = (char *)path + 1;
    } else {
        parent = cwd;
        sptr = (char *)path;
    }
    assert(parent != NULL);

    while (*sptr != 0) {
        len = getfilename(sptr, filename);
        sptr = sptr + len;
        if (*sptr == '/')
            sptr++;
        if (!strcmp(filename, "..")) {
            parent = parent->f_parent;
            continue;
        }
        if (!strcmp(filename, "."))
            continue;
        child = NULL;
        list_for_each_entry(pos, &parent->f_children, f_list)
        {
            if (!strncmp(pos->f_name, filename, len)) {
                child = pos;
                break;
            }
        }
        if (!child) {
            if (!(child = vfs_lookup_in_fs(parent, filename)))
                return NULL;
        } else {
            child->f_count++;
        }
        parent = child;
    }

    return child;
}

static int vfs_read(pid_t pid, int fd, void *buf, size_t size)
{
    message      mess;
    int          status;
    struct file *filp;

    if (!(filp = getfilp(pid, fd)))
        return -1;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    mess.type = MSG_FSREAD;
    mess.m_fsread.buf = buf;
    mess.m_fsread.size = size;
    mess.m_fsread.inode = filp->f_entry->f_ino;
    mess.m_fsread.fsize = filp->f_entry->f_size;
    mess.m_fsread.pread = filp->f_entry->f_pread;
    mess.m_fsread.offset = filp->f_pos;
    if ((status = _sendrecv(filp->f_entry->f_fs_pid, &mess)) != 0) {
        return -1;
    }
    if (mess.retval > 0)
        filp->f_pos += mess.retval;

    return mess.retval;
}

int vfs_write(pid_t pid, int fd, void *buf, size_t size)
{
    message      mess;
    int          status;
    struct file *filp;

    if (!(filp = getfilp(pid, fd)))
        return -1;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    mess.type = MSG_FSWRITE;
    mess.m_fswrite.buf = buf;
    mess.m_fswrite.size = size;
    mess.m_fswrite.inode = filp->f_entry->f_ino;
    mess.m_fswrite.fsize = filp->f_entry->f_size;
    mess.m_fswrite.pwrite = filp->f_entry->f_pwrite;
    mess.m_fswrite.offset = filp->f_pos;
    if ((status = _sendrecv(filp->f_entry->f_fs_pid, &mess)) != 0) {
        return -1;
    }
    if (mess.retval > 0)
        filp->f_pos += mess.retval;

    return mess.retval;
}

static int vfs_open(pid_t pid, const char *path, int oflag, mode_t mode)
{
    struct fentry *   entry;
    struct proc_file *proc_file;
    struct file *     filp;
    int               fd;

    if ((proc_file = map_proc_file(pid)) == NULL) {
        proc_file = init_proc_file(pid);
        assert(proc_file != NULL);
        append_proc_file(proc_file);
    }

    entry = vfs_lookup(proc_file->cwd, path);
    if (entry == NULL) {
        debug("no such file %s\n", path);
        return -1;
    }

    filp = make_filp(entry->f_mode, entry);
    for (fd = 0; fd < NR_FILES; fd++) {
        if (proc_file->filp[fd] == NULL) {
            proc_file->filp[fd] = filp;
            return fd;
        }
    }
    return -1;
}

static int vfs_close(pid_t pid, int fd)
{
    struct file *     filp;
    struct proc_file *proc_file;

    proc_file = map_proc_file(pid);
    assert(proc_file != NULL);
    filp = proc_file->filp[fd];

    if (filp == NULL)
        return 0;
    free_filp(filp);
    proc_file->filp[fd] = NULL;
    return 0;
}

static off_t vfs_lseek(pid_t pid, int fd, off_t offset, int whence)
{
    struct file *filp;

    if ((filp = getfilp(pid, fd)) == NULL)
        return -1;

    switch (whence) {
    case SEEK_SET:
        if (offset < 0)
            return -1;
        filp->f_pos = offset;
        break;
    case SEEK_CUR:
        if (filp->f_pos + offset < 0)
            return -1;
        filp->f_pos = filp->f_pos + offset;
        break;
    case SEEK_END:
        if ((off_t)filp->f_entry->f_size + offset < 0)
            return -1;
        filp->f_pos = filp->f_entry->f_size + offset;
        break;
    default:
        return -1;
    }
    return filp->f_pos;
}

static int getcwdstr(struct fentry *entry, char *buf, int size)
{
    assert(entry != NULL);
    int n, len;

    if (entry == root_entry) {
        len = strlen(entry->f_name);
        if (size < len + 1)  // \0
            return -1;
        strncpy(buf, entry->f_name, len);
        buf[len] = 0;
        return len;
    }
    n = getcwdstr(entry->f_parent, buf, size);
    len = strlen(entry->f_name);
    if (size < len + 2)  // \0和/
        return -1;
    strncpy(buf + n, entry->f_name, len);
    buf[n + len] = '/';
    buf[n + len + 1] = 0;
    return n + len + 1;
}

static int vfs_getcwd(pid_t pid, char *buf, size_t size)
{
    struct proc_file *proc_file;

    if ((proc_file = map_proc_file(pid)) == NULL)
        return -1;

    if (getcwdstr(proc_file->cwd, buf, size) < 0)
        return -1;
    return 0;
}

static int vfs_chdir(pid_t pid, const char *pathname)
{
    struct proc_file *proc_file;
    struct fentry *   newcwd;

    if ((proc_file = map_proc_file(pid)) == NULL)
        return -1;
    if ((newcwd = vfs_lookup(proc_file->cwd, pathname)) == NULL)
        return -1;
    proc_file->cwd->f_count--;
    proc_file->cwd = newcwd;
    return 0;
}

static int vfs_copyfs(pid_t ppid, pid_t pid)
{
    int fd;

    struct proc_file *pproc_file, *proc_file;
    struct file *     pfilp, *filp;

    if ((pproc_file = map_proc_file(ppid)) == NULL) {
        pproc_file = init_proc_file(ppid);
        append_proc_file(pproc_file);
    }

    proc_file = (struct proc_file *)malloc(sizeof(struct proc_file));
    assert(proc_file != NULL);
    memset(proc_file, 0x00, sizeof(struct proc_file));
    proc_file->pid = pid;
    proc_file->cwd = pproc_file->cwd;
    proc_file->cwd->f_count++;

    for (fd = 0; fd < NR_FILES; fd++) {
        if (!(pfilp = pproc_file->filp[fd]))
            continue;
        filp = (struct file *)malloc(sizeof(struct file));
        assert(filp != NULL);
        memcpy(filp, pfilp, sizeof(struct file));
        filp->f_entry->f_count++;
        proc_file->filp[fd] = filp;
    }
    append_proc_file(proc_file);
    return 0;
}

/* exec时重新初始化文件系统信息，保留cwd */
static int vfs_execfs(pid_t pid)
{
    int i;

    struct proc_file *proc_file;
    struct file *     filp;

    if ((proc_file = map_proc_file(pid)) == NULL) {
        proc_file = init_proc_file(pid);
        append_proc_file(proc_file);
        return 0;
    }

    for (i = 0; i < NR_FILES; i++) {
        if ((filp = proc_file->filp[i]) != NULL)
            free_filp(filp);
        proc_file->filp[i] = NULL;
    }

    proc_file->filp[0] = make_filp(stdin->f_mode, stdin);
    proc_file->filp[1] = make_filp(stdout->f_mode, stdout);
    proc_file->filp[2] = make_filp(stderr->f_mode, stderr);
    return 0;
}

static int vfs_freefs(pid_t pid)
{
    int fd;

    struct proc_file *proc_file;
    struct file *     filp;

    if ((proc_file = map_proc_file(pid)) == NULL)
        return 0;

    for (fd = 0; fd < NR_FILES; fd++) {
        filp = proc_file->filp[fd];
        if (!filp)
            continue;
        filp->f_entry->f_count--;
        free(filp);
    }

    unmap_proc_file(pid);
    free(proc_file);
    return 0;
}

static int mount_root(size_t start_lba)
{
    debug("mount root\n", start_lba);
    struct vmount *root_mount;
    message        mess;
    msg_fsmnt *    fsmnt;
    int            status;

    status = _recv(IPC_EXT2, &mess);
    assert(status == 0);
    assert(mess.type == MSG_FSMNT);
    assert(mess.m_fsmnt.type == FSMNT_STEP1 && mess.m_fsmnt.systemid == 0xc);

    mess.retval = start_lba;
    status = _send(IPC_EXT2, &mess);
    assert(status == 0);

    status = _recv(IPC_EXT2, &mess);
    assert(status == 0 && mess.m_fsmnt.type == FSMNT_STEP2);

    fsmnt = &mess.m_fsmnt;
    root_entry = (struct fentry *)malloc(sizeof(struct fentry));
    assert(root_entry != NULL);
    root_entry->f_fs_pid = IPC_EXT2;
    root_entry->f_flags = FE_NORMAL;
    root_entry->f_count = 1;
    root_entry->f_ino = fsmnt->inode;
    root_entry->f_mode = fsmnt->mode;
    root_entry->f_pread = fsmnt->pread;
    root_entry->f_pwrite = fsmnt->pwrite;
    root_entry->f_size = fsmnt->fsize;
    root_entry->f_parent = root_entry;
    strcpy(root_entry->f_name, "/");
    list_head_init(&root_entry->f_children);

    root_mount = (struct vmount *)malloc(sizeof(struct vmount));
    assert(root_mount != NULL);

    root_mount->m_entry = root_entry;
    root_mount->m_fs_pid = IPC_EXT2;
    list_add_tail(&root_mount->list, &mount_head);

    mess.type = MSG_FSMNT;
    mess.retval = 0;
    status = _send(IPC_EXT2, &mess);
    assert(status == 0);

    return 0;
}

/* TODO: 使用内核传入的设备表 */
static struct dev_file dev_table[] = {
    [0] =
        {
            "video",
            IPC_VIDEO,
            0x92,
        },
    [1] =
        {
            "keyboard",
            IPC_KB,
            0x124,
        },
};

static int mount_dev(void)
{
    assert(root_entry != NULL);

    struct fentry *  dev_dir, *dev;
    struct dev_file *dev_file;
    int              i, nr_dev;

    dev_dir = (struct fentry *)malloc(sizeof(struct fentry));
    assert(dev_dir != NULL);

    memset(dev_dir, 0x00, sizeof(struct fentry));
    dev_dir->f_fs_pid = IPC_KERNEL;
    dev_dir->f_flags = FE_DEV;
    dev_dir->f_mode = 0x1b6;
    strcpy(dev_dir->f_name, "dev");
    list_head_init(&dev_dir->f_children);

    list_add_tail(&dev_dir->f_list, &root_entry->f_children);
    root_entry->f_count++;

    nr_dev = sizeof(dev_table) / sizeof(struct dev_file);
    for (i = 0; i < nr_dev; i++) {
        dev_file = &dev_table[i];
        dev = (struct fentry *)malloc(sizeof(struct fentry));
        assert(dev != NULL);
        memset(dev, 0x00, sizeof(struct fentry));
        dev->f_fs_pid = dev_file->driver_pid;
        dev->f_flags = FE_DEV;
        dev->f_ino = i;
        dev->f_count = 1;
        dev->f_mode = dev_file->mode;
        strcpy(dev->f_name, dev_file->name);
        list_head_init(&dev->f_children);
        list_add_tail(&dev->f_list, &dev_dir->f_children);
        dev_dir->f_count++;

        /* TODO: */
        if (dev_file->driver_pid == IPC_VIDEO) {
            stdout = stderr = dev;
        } else if (dev_file->driver_pid == IPC_KB) {
            stdin = dev;
        }
    }
    return 0;
}

static void do_process(void)
{
    while (1) {
        message m;
        long    retval;
        if (_recv(IPC_ALL, &m) == -1) {
            debug("vfs: recv error\n");
        }
        switch (m.type) {
        case MSG_READ:
            retval = vfs_read(m.src, m.m_read.fd, m.m_read.buf, m.m_read.size);
            break;
        case MSG_WRITE:
            retval = vfs_write(m.src, m.m_read.fd, m.m_read.buf, m.m_read.size);
            break;
        case MSG_OPEN:
            retval = vfs_open(m.src, m.m_open.filepath, m.m_open.oflag, m.m_open.mode);
            break;
        case MSG_CLOSE:
            retval = vfs_close(m.src, m.m_close.fd);
            break;
        case MSG_LSEEK:
            retval = vfs_lseek(m.src, m.m_lseek.fd, m.m_lseek.offset, m.m_lseek.whence);
            break;
        case MSG_GETCWD:
            retval = vfs_getcwd(m.src, m.m_getcwd.buf, m.m_getcwd.size);
            break;
        case MSG_CHDIR:
            retval = vfs_chdir(m.src, m.m_chdir.pathname);
            break;
        case MSG_COPYFS:
            retval = vfs_copyfs(m.src, m.m_copyfs.pid);
            break;
        case MSG_EXECFS:
            retval = vfs_execfs(m.src);
            break;
        case MSG_FREEFS:
            retval = vfs_freefs(m.src);
            break;
        default:
            debug("vfs: unsupport message\n");
            retval = -1;
        }
        m.retval = retval;
        // debug("vfs ret %d src %d\n", m.ret, m.src);
        if (_send(m.src, &m) == -1) {
            debug("vfs: send error\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int    i;
    size_t start_lba;

    /* init __file_map */
    for (i = 0; i < __FILE_MAP_SIZE; i++) list_head_init(&__file_map[i]);
    list_head_init(&mount_head);

    struct mbr_sector *mbr = (struct mbr_sector *)malloc(512);
    if (bdev_read(0, mbr, 512) != 0) {
        debug("vfs storage read error\n");
        goto faild;
    }

    if (mbr->magic != 0xaa55) {
        debug("mbr read error\n");
        goto faild;
    }

    start_lba = ~(size_t)0;
    for (i = 0; i < 4; i++) {
        struct mbr_dpte *dpte = &mbr->dpte[i];
        if (dpte->flags == 0x80) {
            start_lba = dpte->start_LBA;
            break;
        }
    }
    if (start_lba == ~(size_t)0) {
        debug("read start lba error\n");
        goto faild;
    }

    mount_root(start_lba);
    mount_dev();

    do_process();

faild:
    debug("vfs init faild\n");
    while (1) nop();
    return 0;
}
