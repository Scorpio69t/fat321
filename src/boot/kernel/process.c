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
#include <feng/type.h>

/**
 * 获取用户栈的esp
 */
static u32 get_esp(struct task_struct *new, u32 esp)
{
    return (((u32) new->stack &(~(USER_STACK_SIZE - 1))) | (esp & (USER_STACK_SIZE - 1)));
}

int setup_thread(struct task_struct *p, struct pt_regs *regs, int flags)
{
    struct pt_regs *newregs;

    p->thread.esp0 = (unsigned long)kernel_stack_top(p);
    newregs = get_pt_regs(p);
    memcpy(newregs, regs, sizeof(*newregs));
    newregs->eax = 0;
    if (p->flags & PF_KTHREAD) {
        newregs->esp = (unsigned long)newregs;
        p->thread.eip = (unsigned long)kernel_thread_ret;
    } else {
        newregs->esp = get_esp(p, regs->esp);
        p->thread.eip = (unsigned long)ret_from_fork;
    }
    p->thread.esp = (unsigned long)newregs;
    return 0;
}

pid_t _kernel_thread(struct pt_regs *regs, int (*fn)(void), void *args, unsigned long flags)
{
    memset(regs, 0, sizeof(struct pt_regs));
    regs->cs = SELECTOR_FLAT_C;
    regs->ds = regs->es = regs->ss = regs->fs = SELECTOR_FLAT_RW;
    regs->gs = SELECTOR_VIDEO;
    regs->eip = (unsigned long)kernel_thread_ret;
    regs->ebx = (unsigned long)fn;
    return do_fork(flags | CLONE_VM | CLONE_FS, 0, regs, 0);
}
