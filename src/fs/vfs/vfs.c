#include "vfs.h"

#include <malloc.h>
#include <stdio.h>
#include <sys/list.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "blkdev.h"

static struct ventry root;

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
        printf("process_stdin error\n");
        return -1;
    }
    return kb_m.ret;
}

int process_stdout(message *m)
{
    message video_m;
    video_m.type = MSG_WRITE;
    video_m.m_write.buf = m->m_write.buf;
    video_m.m_write.size = m->m_write.size;
    if (_sendrecv(IPC_VIDEO, &video_m) != 0) {
        printf("process_stdout error\n");
        return -1;
    }
    return video_m.ret;
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

static int msg_type_set[] = {
    MSG_READ,
    MSG_WRITE,
};

int check_msg_type(int type)
{
    int i, len;

    len = sizeof(msg_type_set) / sizeof(int);

    for (i = 0; i < len; i++)
        if (type == msg_type_set[i])
            return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    /* init __file_map */
    // for (i = 0; i < __FILE_MAP_SIZE; i++) list_head_init(&__file_map[i]);

    // struct mbr_sector *mbr = (struct mbr_sector *)malloc(512);

    debug("vfs main\n");
    while (1) {
        message m;
        long    ret;
        if (_recv(IPC_ALL, &m) == -1) {
            printf("vfs: recv error\n");
        }
        if (!check_msg_type(m.type)) {
            printf("vfs: invalid message type\n");
            continue;
        }
        if (m.type == MSG_READ)
            ret = vfs_read(&m);
        else if (m.type == MSG_WRITE)
            ret = vfs_write(&m);
        else {
            printf("vfs: unsupport message\n");
            ret = -1;
        }
        m.ret = ret;
        // debug("vfs ret %d src %d\n", m.ret, m.src);
        if (_send(m.src, &m) == -1) {
            printf("vfs: send error\n");
        }
    }
    return 0;
}
