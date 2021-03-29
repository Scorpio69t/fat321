#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/process.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/malloc.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/types.h>

int copy_context(struct proc_struct *p, struct pt_regs *regs, int flags)
{
    struct pt_regs *newregs;
    newregs = (struct pt_regs *)kernel_stack_top(p) - 1;
    *newregs = *regs;

    newregs->rsp = (uint64)newregs;
    newregs->rax = 0;

    p->context.rsp0 = (uint64)newregs; /* ignore some stack space */
    p->context.rsp = (uint64)newregs;
    p->context.rip = (uint64)ret_from_fork;

    return 0;
}
