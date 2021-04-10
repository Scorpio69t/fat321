#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/unistd.h>
#include <kernel/dirent.h>
#include <kernel/fcntl.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/syscalls.h>
#include <kernel/types.h>
#include <kernel/unistd.h>
#include <kernel/fork.h>
#include <stdarg.h>

long sys_send(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    to = regs->rdi;
    long     status;

    switch (to) {
    case IPC_KERNEL:
        status = process_kernel_message(msg);
        break;
    default:
        status = do_send(regs, to, msg);
        break;
    }
    return status;
}

long sys_recv(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    from = regs->rdi;
    long     status;

    status = do_recv(regs, from, msg);
    return status;
}

int deal_mm(frame_t *regs, message *msg)
{
    switch (msg->type) {
    case MSG_BRK:
        msg->ret = do_brk(msg->m_brk.addr);
        break;
    case MSG_FORK:
        msg->ret = do_fork(regs);
    default:
        break;
    }
    return 0;
}

long sys_sendrecv(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    who = regs->rdi;
    long     status;

    switch (who) {
    case IPC_MM:
        status = deal_mm(regs, msg);
        break;
    default:
        status = do_sendrecv(regs, who, msg);
        break;
    }
    return status;
}

long sys_debug(frame_t *regs)
{
    printk("%s", regs->rdi);
    return 0;
}

void pre_syscall(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    msg->src = current->pid;
    switch (msg->type) {
    case MSG_READ:
        msg->m_read.buf = kmap(msg->m_read.buf);
        break;
    case MSG_WRITE:
        msg->m_write.buf = kmap(msg->m_write.buf);
        break;
    case MSG_DISK:
        msg->m_disk.buf = kmap(msg->m_disk.buf);
        break;
    default:
        break;
    }
}
