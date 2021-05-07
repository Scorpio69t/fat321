#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/dentry.h>
#include <sys/fs.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

static struct fs_ops *fs_ops;

static long part_position(const char *fsname)
{
    message m;

    if (_kmap((void **)&fsname, NULL, NULL) != 0) {
        panic("part_position kmap failed\n");
        return -1;
    }

    m.type = MSG_BDEV_PART;
    m.m_bdev_part.fsname = fsname;
    if (_sendrecv(IPC_DISK, &m) != 0 && m.retval < 0) {
        panic("part_position send m_bdev_part message failed\n");
        return -1;
    }
    return m.retval;
}

static int mount_fs(const char *pmnt, unsigned long position)
{
    message m;
    struct dentry *dentry, *dtmp;

    if (!(dentry = (struct dentry *)malloc(sizeof(struct dentry))))
        return -1;
    fs_ops->fs_init(position, dentry);
    dtmp = dentry;
    if (_kmap((void **)&pmnt, (void **)&dtmp, NULL) != 0) {
        panic("mount_fs kmap failed\n");
        goto failed;
    }

    m.type = MSG_FSMNT;
    m.m_fs_mnt.pmnt = pmnt;
    m.m_fs_mnt.dentry = dtmp;
    if (_sendrecv(IPC_VFS, &m) != 0 || m.retval != 0) {
        panic("mount_fs fs mount failed\n");
        goto failed;
    }
    return 0;
failed:
    free(dentry);
    return -1;
}

int run_fs(const char *fsname, const char *pmnt, struct fs_ops *ops)
{
    message m;
    long position;
    int retval;

    fs_ops = ops;

    if ((position = part_position(fsname)) < 0)
        return -1;

    if (mount_fs(pmnt, (unsigned long)position) < 0) {
        panic("mount fs failed\n");
        return -1;
    }

    while (1) {
        assert(_recv(IPC_VFS, &m) == 0);

        switch (m.type) {
        case MSG_FSREAD:
            retval = fs_ops->fs_read(m.m_fs_read.inode, m.m_fs_read.buf, m.m_fs_read.offset, m.m_fs_read.size);
            break;
        case MSG_FSWRITE:
            retval = fs_ops->fs_write(m.m_fs_write.inode, m.m_fs_write.buf, m.m_fs_write.offset, m.m_fs_write.size);
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
        default:
            retval = -1;
        }

        m.retval = retval;
        assert(_send(IPC_VFS, &m) == 0);
    }
    return 0;
}
