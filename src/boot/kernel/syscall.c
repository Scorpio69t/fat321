#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/unistd.h>
#include <kernel/dirent.h>
#include <kernel/fcntl.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/types.h>
#include <kernel/unistd.h>
#include <stdarg.h>

uint64 sys_send(frame_t *regs)
{
    message *msg = (message *)regs->rsi;
    if (regs->rdi == IPC_INTR) {
        printk("irq_no: %d\n", msg->m_irq.irq_no);
        register_irq(msg->m_irq.irq_no, current->pid);
    }
    return 0;
}

uint64 sys_recv(frame_t *regs)
{
    proc_t *proc = current;
    proc->state = PROC_RECEIVING;
    proc->wait = IPC_INTR;

    schedule();

    message *msg = (message *)regs->rsi;
    memcpy(msg, &proc->msg, sizeof(message));

    return 0;
}

uint64 sys_sendrecv(frame_t *regs)
{
    return 0;
}

long sys_debug(frame_t *regs)
{
    printk("pid: %d %s", current->pid, regs->rdi);
    return 0;
}
