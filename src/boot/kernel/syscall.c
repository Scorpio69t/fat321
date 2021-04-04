#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/unistd.h>
#include <kernel/dirent.h>
#include <kernel/fcntl.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/types.h>
#include <kernel/unistd.h>
#include <stdarg.h>

void sys_send(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    to = regs->rdi;

    switch (to) {
    case IPC_INTR:
        if (msg->m_irq.type == IRQ_REGISTER)
            regs->rax = register_irq(msg->m_irq.irq_no, current->pid);
        else if (msg->m_irq.type == IRQ_UNREGISTER)
            regs->rax = unregister_irq(msg->m_irq.irq_no);
        else
            regs->rax = -1;
        break;
    default:
        regs->rax = do_send(regs, to, msg);
        break;
    }
}

void sys_recv(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    from = regs->rdi;
    switch (from) {
    case IPC_INTR:
        regs->rax = nt_recv(regs, from, msg);
        break;
    case IPC_BOTH:
        regs->rax = nt_recv(regs, from, msg);
        break;
    default:
        regs->rax = do_recv(regs, from, msg);
        break;
    }
}

void sys_sendrecv(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    to = regs->rdi;
    regs->rax = do_sendrecv(regs, to, msg);
}

void sys_debug(frame_t *regs)
{
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
