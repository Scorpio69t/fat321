#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/process.h>
#include <feng/bugs.h>
#include <feng/fork.h>
#include <feng/gfp.h>
#include <feng/kernel.h>
#include <feng/linkage.h>
#include <feng/malloc.h>
#include <feng/sched.h>
#include <feng/slab.h>
#include <feng/types.h>

int copy_thread(struct task_struct *p, struct pt_regs *regs, int flags)
{
    struct pt_regs *newregs;
    newregs = (struct pt_regs *)kernel_stack_top(p) - 1;
    *newregs = *regs;

    newregs->rsp = (uint64)newregs;
    newregs->rax = 0;

    p->thread.rsp0 = (uint64)newregs; /* ignore some stack space */
    p->thread.rsp = (uint64)newregs;
    p->thread.rip = (uint64)ret_from_fork;

    return 0;
}
