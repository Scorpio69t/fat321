#include "vfs.h"

#include <bugs.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/list.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "blkdev.h"

struct list_head      mount_head;
static struct fentry *root_entry;

#define __FILE_MAP_SIZE 1024
static struct list_head __file_map[__FILE_MAP_SIZE];

file_t *map_file_t(pid_t pid)
{
    struct list_head *head;
    file_t *          pos, *entry;

    head = &__file_map[pid % __FILE_MAP_SIZE];
    list_for_each_entry(pos, head, list)
    {
        if (pos->pid == pid) {
            entry = pos;
            break;
        }
    }
    return entry;
}

int append_fild_t(pid_t pid, file_t *fsp)
{
    struct list_head *head;
    file_t *          pos;

    head = &__file_map[pid % __FILE_MAP_SIZE];
    list_for_each_entry(pos, head, list)
    {
        if (pos->pid == pid)
            return -1;
    }
    list_add_tail(&fsp->list, head);
    return 0;
}

int create_files(pid_t pid)
{
    return 0;
}

int copy_files(pid_t parent, pid_t child)
{
    return 0;
}

int process_stdin(message *m)
{
    message kb_m;
    kb_m.type = MSG_READ;
    kb_m.m_read.buf = m->m_read.buf;
    kb_m.m_read.size = m->m_read.size;
    if (_sendrecv(IPC_KB, &kb_m) != 0) {
        debug("process_stdin error\n");
        return -1;
    }
    return kb_m.retval;
}

int process_stdout(message *m)
{
    message video_m;
    video_m.type = MSG_WRITE;
    video_m.m_write.buf = m->m_write.buf;
    video_m.m_write.size = m->m_write.size;
    if (_sendrecv(IPC_VIDEO, &video_m) != 0) {
        debug("process_stdout error\n");
        return -1;
    }
    return video_m.retval;
}

int vfs_read(message *m)
{
    int fd = m->m_read.fd;
    if (fd == 0)
        return process_stdin(m);
    return -1;
}

int vfs_write(message *m)
{
    int fd = m->m_write.fd;

    if (fd == 1 || fd == 2)
        return process_stdout(m);
    return -1;
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

    return child;
}

struct fentry *vfs_lookup(const char *path)
{
    char *         sptr;
    int            len;
    struct fentry *parent, *child, *pos;
    char           filename[256];

    if (!strncmp("/", path, 1)) {
        parent = root_entry;
        sptr = (char *)path + 1;
    }
    assert(parent != NULL);

    while (*sptr != 0) {
        len = getfilename(sptr, filename);
        sptr = sptr + len;
        if (*sptr == '/')
            sptr++;

        child = NULL;
        list_for_each_entry(pos, &parent->f_children, f_list)
        {
            if (!strncmp(pos->f_name, filename, len)) {
                child = pos;
                break;
            }
        }

        if (!child) {
            child = vfs_lookup_in_fs(parent, filename);
            if (!child)
                return NULL;
        } else {
            child->f_count++;
        }

        parent = child;
    }

    return child;
}

static int msg_type_set[] = {
    MSG_READ,
    MSG_WRITE,
};

static int mount_root(size_t start_lba)
{
    debug("mount root\n", start_lba);
    struct vmount *root_mount;
    message        mess;
    msg_fsmnt *    fsmnt;
    int            status;

    status = _recv(IPC_FAT, &mess);
    assert(status == 0);
    assert(mess.type == MSG_FSMNT);
    assert(mess.m_fsmnt.type == FSMNT_STEP1 && mess.m_fsmnt.systemid == 0xc);

    mess.retval = start_lba;
    status = _send(IPC_FAT, &mess);
    assert(status == 0);

    status = _recv(IPC_FAT, &mess);
    assert(status == 0 && mess.m_fsmnt.type == FSMNT_STEP2);

    fsmnt = &mess.m_fsmnt;
    root_entry = (struct fentry *)malloc(sizeof(struct fentry));
    assert(root_entry != NULL);
    root_entry->f_fs_pid = IPC_FAT;
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
    root_mount->m_fs_pid = IPC_FAT;
    list_add_tail(&root_mount->list, &mount_head);

    mess.type = MSG_FSMNT;
    mess.retval = 0;
    status = _send(IPC_FAT, &mess);
    assert(status == 0);

    return 0;
}

int check_msg_type(int type)
{
    int i, len;

    len = sizeof(msg_type_set) / sizeof(int);

    for (i = 0; i < len; i++)
        if (type == msg_type_set[i])
            return 1;
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
        if (!check_msg_type(m.type)) {
            debug("vfs: invalid message type\n");
            continue;
        }
        if (m.type == MSG_READ)
            retval = vfs_read(&m);
        else if (m.type == MSG_WRITE)
            retval = vfs_write(&m);
        else {
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

void test()
{
    struct fentry *e = vfs_lookup("/home/a.txt");
    assert(e != NULL);
    debug("inode %d fsize %d\n", e->f_ino, e->f_size);

    message mess;
    char *  buffer;
    int     status;

    buffer = (char *)malloc(512);

    mess.type = MSG_FSREAD;
    mess.m_fsread.buf = buffer;
    mess.m_fsread.size = 512;
    mess.m_fsread.inode = e->f_ino;
    mess.m_fsread.fsize = e->f_size;
    mess.m_fsread.pread = e->f_pread;
    mess.m_fsread.offset = 0;
    status = _sendrecv(e->f_fs_pid, &mess);
    assert(status == 0);

    buffer[mess.retval] = 0;
    debug("read size %d  %s\n", mess.retval, buffer);
}

int main(int argc, char *argv[])
{
    int    i;
    size_t start_lba;

    /* init __file_map */
    for (i = 0; i < __FILE_MAP_SIZE; i++) list_head_init(&__file_map[i]);
    list_head_init(&mount_head);

    struct mbr_sector *mbr = (struct mbr_sector *)malloc(512);
    if (storage_read(1, 0, mbr) != 0) {
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
        if (dpte->type == 0xc && dpte->flags == 0x80) {
            start_lba = dpte->start_LBA;
            break;
        }
    }
    if (start_lba == ~(size_t)0) {
        debug("read start lba error\n");
        goto faild;
    }

    mount_root(start_lba);
    debug("vfs main\n");

    test();

    do_process();

faild:
    debug("vfs init faild\n");
    while (1) nop();
    return 0;
}
