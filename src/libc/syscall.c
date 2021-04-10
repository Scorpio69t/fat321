#include <sys/ipc.h>
#include <sys/syscall.h>

int _syscall(int who, int type, message *msg)
{
    int status;

    msg->type = type;
    if ((status = _sendrecv(who, msg)) != 0) {
        return -1;
    }
    return msg->ret;
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
    message m;
    m.m_read.fd = fd;
    m.m_read.buf = buf;
    m.m_read.size = nbytes;
    return _syscall(IPC_VFS, MSG_READ, &m);
}

ssize_t write(int fd, void *buf, size_t nbytes)
{
    message m;
    m.m_write.fd = fd;
    m.m_write.buf = buf;
    m.m_write.size = nbytes;
    return _syscall(IPC_VFS, MSG_WRITE, &m);
}

int brk(void *addr)
{
    message msg;

    msg.m_brk.addr = addr;
    return _syscall(IPC_MM, MSG_BRK, &msg);
}

void *sbrk(long size)
{
    extern void *_brk;
    void *       old_brk, *new_brk;

    old_brk = _brk;
    new_brk = old_brk + size;
    if (old_brk == new_brk)
        return old_brk;
    if (brk(new_brk) != 0) {
        return NULL;
    }
    _brk = new_brk;
    return old_brk;
}

int register_irq(int no_intr)
{
    message m;
    m.type = MSG_IRQ;
    m.m_irq.type = IRQ_REGISTER;
    m.m_irq.irq_no = no_intr;
    if (_send(IPC_KERNEL, &m) != 0)
        return -1;
    return m.ret;
}

int unregister_irq(int no_intr)
{
    message m;
    m.type = MSG_IRQ;
    m.m_irq.type = IRQ_UNREGISTER;
    m.m_irq.irq_no = no_intr;
    if (_send(IPC_KERNEL, &m) != 0)
        return -1;
    return m.ret;
}

int storage_read(unsigned char nsect, unsigned long sector, void *buf)
{
    message m;
    m.type = MSG_DISK;
    m.m_disk.type = DISK_READ;
    m.m_disk.nsect = nsect;
    m.m_disk.sector = sector;
    m.m_disk.buf = buf;
    if (_sendrecv(IPC_DISK, &m) != 0)
        return -1;
    return m.ret;
}

int storage_write(unsigned char nsect, unsigned long sector, void *buf)
{
    message m;
    m.type = MSG_DISK;
    m.m_disk.type = DISK_WRITE;
    m.m_disk.nsect = nsect;
    m.m_disk.sector = sector;
    m.m_disk.buf = buf;
    if (_sendrecv(IPC_DISK, &m) != 0)
        return -1;
    return m.ret;
}
