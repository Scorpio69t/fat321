#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <kernel/fork.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/syscalls.h>
#include <kernel/types.h>
#include <stdarg.h>

long sys_send(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    to = regs->rdi;
    long     status;

    status = do_send(regs, to, msg);
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

long sys_sendrecv(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    pid_t    who = regs->rdi;
    long     status;

    switch (who) {
    case IPC_KERNEL:
        status = process_kernel_message(regs, msg);
        break;
    case IPC_MM:
        status = process_mm_message(regs, msg);
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
}
