#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/dentry.h>
#include <sys/fs.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

int run_fs(struct fs_ops *fs_ops)
{
    message m;
    int retval;

    while (1) {
        assert(_recv(IPC_VFS, &m) == 0);

        switch (m.type) {
        case MSG_FSREAD:
            retval = fs_ops->fs_read(m.m_fs_read.inode, m.m_fs_read.buf, m.m_fs_read.offset, m.m_fs_read.size);
            break;
        case MSG_FSWRITE:
            retval = fs_ops->fs_write(m.m_fs_write.inode, m.m_fs_write.buf, m.m_fs_write.offset, m.m_fs_write.size);
            break;
        case MSG_MKDIR:
            retval = fs_ops->fs_mkdir(m.m_fs_mkdir.ino, m.m_fs_mkdir.name, m.m_fs_mkdir.mode);
            break;
        case MSG_FSLOOKUP:
            retval = fs_ops->fs_lookup(m.m_fs_lookup.filename, m.m_fs_lookup.pino, m.m_fs_lookup.dentry);
            break;
        case MSG_FSSTAT:
            retval = fs_ops->fs_stat(m.m_fs_stat.inode, m.m_fs_stat.buf);
            break;
        case MSG_FSGETDENTS:
            retval = fs_ops->fs_getdents(m.m_fs_getdents.inode, m.m_fs_getdents.buf, m.m_fs_getdents.offset,
                                         m.m_fs_getdents.nbytes);
            break;
        case MSG_FSMNT:
            retval = fs_ops->fs_mount(m.m_fs_mnt.part, m.m_fs_mnt.dentry);
            break;
        default:
            retval = -1;
        }
        m.retval = retval;
        assert(_send(IPC_VFS, &m) == 0);
    }
    return 0;
}
