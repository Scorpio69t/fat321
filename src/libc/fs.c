#include <assert.h>
#include <stdio.h>
#include <sys/fs.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

static struct fs_ops *fs_ops;

static int mount_fs()
{
    message         m;
    unsigned long   start_lba;
    struct fs_entry entry;

    m.type = MSG_FSMNT;
    m.m_fsmnt.type = FSMNT_STEP1;
    m.m_fsmnt.systemid = 0xc;
    if (_sendrecv(IPC_VFS, &m) != 0) {
        panic("mount_fs sendrecv error\n");
        return -1;
    }
    if (m.retval < 0) {
        panic("m.ret error\n");
        return -1;
    }

    start_lba = m.retval * 512;

    fs_ops->fs_init(start_lba, &entry);

    m.type = MSG_FSMNT;
    m.m_fsmnt.type = FSMNT_STEP2;
    m.m_fsmnt.inode = entry.inode;
    m.m_fsmnt.pread = entry.pread;
    m.m_fsmnt.pwrite = entry.pwrite;
    m.m_fsmnt.mode = entry.mode;
    m.m_fsmnt.fsize = entry.fsize;

    if (_sendrecv(IPC_VFS, &m) != 0) {
        panic("mount_fs sendrecv error\n");
        return -1;
    }
    if (m.retval < 0) {
        panic("m.ret error\n");
        return -1;
    }

    debug("mount fs\n");

    return 0;
}

static void do_lookup(message *msg)
{
    int             retval;
    struct fs_entry p_entry, entry;

    p_entry.inode = msg->m_fslookup.p_inode;
    p_entry.pread = msg->m_fslookup.p_pread;
    retval = fs_ops->fs_lookup(msg->m_fslookup.filename, &p_entry, &entry);
    if (retval != 0) {
        msg->retval = retval;
        return;
    }

    msg->retval = 0;
    msg->m_fslookup.inode = entry.inode;
    msg->m_fslookup.pread = entry.pread;
    msg->m_fslookup.pwrite = entry.pwrite;
    msg->m_fslookup.mode = entry.mode;
    msg->m_fslookup.fsize = entry.fsize;
}

static void do_read(message *msg)
{
    int             retval;
    struct fs_entry entry;

    entry.inode = msg->m_fsread.inode;
    entry.fsize = msg->m_fsread.fsize;
    entry.pread = msg->m_fsread.pread;

    retval = fs_ops->fs_read(&entry, msg->m_fsread.buf, msg->m_fsread.offset, msg->m_fsread.size);

    msg->retval = retval;
}

static void do_stat(message *msg)
{
    int             retval;
    struct fs_entry entry;

    entry.inode = msg->m_fsstat.inode;
    retval = fs_ops->fs_stat(&entry, msg->m_fsstat.buf);
    msg->retval = retval;
}

int run_fs(struct fs_ops *ops)
{
    message m;
    int     retval;

    fs_ops = ops;

    if (mount_fs()) {
        panic("mount fs failed\n");
        return -1;
    }

    while (1) {
        assert(_recv(IPC_VFS, &m) == 0);

        retval = 0;
        switch (m.type) {
        case MSG_FSREAD:
            do_read(&m);
            break;
        case MSG_FSWRITE:
            break;
        case MSG_FSLOOKUP:
            do_lookup(&m);
            break;
        case MSG_FSSTAT:
            do_stat(&m);
            break;
        default:
            retval = -1;
        }

        if (retval != 0)
            m.retval = retval;
        assert(_send(IPC_VFS, &m) == 0);
    }
    return 0;
}
