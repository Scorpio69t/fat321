#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

int _syscall(int who, int type, message *msg)
{
    int status;

    msg->type = type;
    if ((status = _sendrecv(who, msg)) != 0) {
        return -1;
    }
    return msg->retval;
}

int _kmap(void **addr1, void **addr2, void **addr3)
{
    message m;

    memset(&m, 0x00, sizeof(message));
    if (addr1)
        m.m_kmap.addr1 = *addr1;
    if (addr2)
        m.m_kmap.addr2 = *addr2;
    if (addr3)
        m.m_kmap.addr3 = *addr3;

    if (_syscall(IPC_MM, MSG_KMAP, &m) != 0)
        return -1;

    if (addr1)
        *addr1 = m.m_kmap.addr1;
    if (addr2)
        *addr2 = m.m_kmap.addr2;
    if (addr3)
        *addr3 = m.m_kmap.addr3;
    return 0;
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
    message m;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    m.m_read.fd = fd;
    m.m_read.buf = buf;
    m.m_read.size = nbytes;
    return _syscall(IPC_VFS, MSG_READ, &m);
}

ssize_t write(int fd, void *buf, size_t nbytes)
{
    message m;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    m.m_write.fd = fd;
    m.m_write.buf = buf;
    m.m_write.size = nbytes;
    return _syscall(IPC_VFS, MSG_WRITE, &m);
}

int open(const char *path, int oflag, ... /* mode_t mode */)
{
    message m;
    va_list args;

    if (_kmap((void **)&path, NULL, NULL) != 0)
        return -1;
    m.m_open.filepath = (char *)path;
    m.m_open.oflag = oflag;
    if (oflag & O_CREAT) {
        va_start(args, oflag);
        m.m_open.mode = va_arg(args, mode_t);
        va_end(args);
    }
    return _syscall(IPC_VFS, MSG_OPEN, &m);
}

int close(int fd)
{
    message m;

    m.m_close.fd = fd;
    return _syscall(IPC_VFS, MSG_CLOSE, &m);
}

off_t lseek(int fd, off_t offset, int whence)
{
    message m;

    m.m_lseek.fd = fd;
    m.m_lseek.offset = offset;
    m.m_lseek.whence = whence;
    return _syscall(IPC_VFS, MSG_LSEEK, &m);
}

pid_t fork(void)
{
    message m;
    return _syscall(IPC_MM, MSG_FORK, &m);
}

void exit(int status)
{
    message m;
    m.m_exit.status = status;
    _syscall(IPC_KERNEL, MSG_EXIT, &m);
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    message m;

    m.m_execve.pathname = pathname;
    m.m_execve.argv = argv;
    m.m_execve.envp = envp;
    return _syscall(IPC_MM, MSG_EXECVE, &m);
}

int brk(void *addr)
{
    message msg;

    msg.m_brk.addr = (unsigned long)addr;
    return _syscall(IPC_MM, MSG_BRK, &msg);
}

pid_t wait(int *statloc)
{
    message m;
    int status;

    if ((status = _syscall(IPC_KERNEL, MSG_WAIT, &m)) <= 0)
        return status;
    if (statloc)
        *statloc = m.m_wait.statloc;
    return status;
}

void *sbrk(long size)
{
    extern void *_brk;
    void *old_brk, *new_brk;

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

pid_t getpid(void)
{
    message m;

    return _syscall(IPC_KERNEL, MSG_GETPID, &m);
}

char *getcwd(char *buf, size_t size)
{
    message m;
    char *oldbuf;

    oldbuf = buf;
    if (_kmap((void **)&buf, NULL, NULL) != 0)
        return NULL;
    m.m_getcwd.buf = buf;
    m.m_getcwd.size = size;
    if (_syscall(IPC_VFS, MSG_GETCWD, &m) != 0)
        return NULL;
    return oldbuf;
}

int chdir(const char *pathname)
{
    message m;

    if (_kmap((void **)&pathname, NULL, NULL) != 0)
        return -1;
    m.m_chdir.pathname = pathname;
    return _syscall(IPC_VFS, MSG_CHDIR, &m);
}

int stat(const char *pathname, struct stat *buf)
{
    message m;

    if (_kmap((void **)&pathname, (void **)&buf, NULL) != 0)
        return -1;
    m.m_stat.buf = buf;
    m.m_stat.pathname = pathname;
    return _syscall(IPC_VFS, MSG_STAT, &m);
}

int getdents(int fd, struct dirent *dirp, size_t nbytes)
{
    message m;

    if (_kmap((void **)&dirp, NULL, NULL) != 0)
        return 0;
    m.m_getdents.fd = fd;
    m.m_getdents.dirp = dirp;
    m.m_getdents.count = nbytes;
    return _syscall(IPC_VFS, MSG_GETDENTS, &m);
}

int register_irq(int no_intr)
{
    message m;
    m.type = MSG_IRQ;
    m.m_irq.type = IRQ_REGISTER;
    m.m_irq.irq_no = no_intr;
    if (_sendrecv(IPC_KERNEL, &m) != 0)
        return -1;
    return m.retval;
}

int unregister_irq(int no_intr)
{
    message m;
    m.type = MSG_IRQ;
    m.m_irq.type = IRQ_UNREGISTER;
    m.m_irq.irq_no = no_intr;
    if (_sendrecv(IPC_KERNEL, &m) != 0)
        return -1;
    return m.retval;
}

int bdev_read(unsigned long pos, void *buf, size_t size)
{
    message m;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    m.type = MSG_BDEV_TRANSFER;
    m.m_bdev_transfer.buffer = buf;
    m.m_bdev_transfer.pos = pos;
    m.m_bdev_transfer.size = size;
    m.m_bdev_transfer.write = 0;
    if (_sendrecv(IPC_DISK, &m) != 0)
        return -1;
    return m.retval;
}

int bdev_write(unsigned long pos, void *buf, size_t size)
{
    message m;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    m.type = MSG_BDEV_TRANSFER;
    m.m_bdev_transfer.buffer = buf;
    m.m_bdev_transfer.pos = pos;
    m.m_bdev_transfer.size = size;
    m.m_bdev_transfer.write = 1;
    if (_sendrecv(IPC_DISK, &m) != 0)
        return -1;
    return m.retval;
}

long bdev_part_read(int part, unsigned long pos, void *buf, size_t size)
{
    message m;

    if (_kmap(&buf, NULL, NULL) != 0)
        return -1;
    m.type = MSG_BDEV_PART;
    m.m_bdev_part.part = part;
    m.m_bdev_part.pos = pos;
    m.m_bdev_part.buffer = buf;
    m.m_bdev_part.size = size;
    if (_sendrecv(IPC_DISK, &m) != 0)
        return -1;
    return m.retval;
}