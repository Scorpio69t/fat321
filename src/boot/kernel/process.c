#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/process.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/types.h>

int copy_context(struct proc_struct *p, struct pt_regs *regs, int flags)
{
    struct pt_regs *newregs;
    newregs = (struct pt_regs *)kernel_stack_top(p) - 1;
    *newregs = *regs;

    newregs->rsp = (uint64)newregs; /* 只对内核进程有用 */
    newregs->rax = 0;

    p->context.rsp0 = (uint64)newregs; /* ignore some stack space */
    p->context.rsp = (uint64)newregs;
    p->context.rip = (uint64)ret_from_fork;

    return 0;
}

int setup_module_context(proc_t *proc, uint64 entry)
{
    struct pt_regs *newregs;
    newregs = (struct pt_regs *)kernel_stack_top(proc) - 1;
    memset(newregs, 0x00, sizeof(newregs));

    newregs->rip = entry;
    newregs->cs = USER_CODE_DESC;
    newregs->eflags = (1 << 9); /* enable interrupt */
    newregs->rsp = proc->mm.end_stack - 8;
    newregs->ss = USER_DATA_DESC;

    proc->context.rsp0 = (uint64)newregs;
    proc->context.rsp = (uint64)newregs;
    proc->context.rip = (uint64)ret_from_fork;
    return 0;
}
