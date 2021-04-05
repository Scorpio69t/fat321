#include <sys/ipc.h>
#include <sys/syscall.h>

int _syscall(int who, int type, message *msg)
{
    int status;

    msg->type = type;
    if ((status = _sendrecv(who, msg)) != 0) {
        return status;
    }
    return msg->type;
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

int register_irq(int no_intr) {}
