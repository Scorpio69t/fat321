#include <boot/cpu.h>
#include <boot/io.h>
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
    return 0;
}

uint64 sys_recv(frame_t *regs)
{
    return 0;
}

uint64 sys_sendrecv(frame_t *regs)
{
    return 0;
}

long sys_debug(frame_t *regs)
{
    printk("pid: %d %s\n", current->pid, regs->rdi);
    return 0;
}
