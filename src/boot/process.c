#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/memory.h>
#include <boot/process.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/ipc.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/types.h>

int copy_context(struct proc_struct *p, frame_t *regs)
{
    frame_t *newregs;
    newregs = (frame_t *)kernel_stack_top(p) - 1;
    *newregs = *regs;

    p->context.rsp0 = (uint64)newregs; /* ignore some stack space */
    p->context.rsp = (uint64)newregs;
    p->context.rip = (uint64)ret_from_fork;

    return 0;
}

int setup_proc_context(proc_t *proc, uint64 entry, uint64 stack_bottom)
{
    frame_t *newregs;
    newregs = (frame_t *)kernel_stack_top(proc) - 1;
    memset(newregs, 0x00, sizeof(newregs));

    newregs->rip = entry;
    newregs->cs = USER_CODE_DESC;
    newregs->eflags = (1 << 9) | (3 << 12); /* IF, IOPL */
    newregs->rsp = stack_bottom;
    newregs->ss = USER_DATA_DESC;

    proc->context.rsp0 = (uint64)newregs;
    proc->context.rsp = (uint64)newregs;
    proc->context.rip = (uint64)exec_ret;
    return 0;
}

/**
 * 获取当前进程控制块的地址
 *
 * 由于进程控制块占一个页，每个页都是4k对其的，所以将%esp低12位变为零便是当前进程的进程控制块
 * 地址
 */
inline struct proc_struct *__current(void)
{
    struct proc_struct *cur;
    asm volatile("andq %%rsp, %0" : "=r"(cur) : "0"(~((uint64)KERNEL_STACK_SIZE - 1)));
    return cur;
}

/**
 * __switch_to - 进程切换的cpu上下文切换
 * @prev: 当前进程的进程控制块指针 in eax
 * @next: 下一个进程的进程控制块指针 in edx
 */

struct proc_struct *__switch_to(struct proc_struct *prev, struct proc_struct *next)
{
    init_tss.rsp0 = next->context.rsp0;

    uint64 pgd = get_pgd();
    if (next->flags & PF_KTHREAD) {
        if (pgd != kinfo.global_pgd_start) {
            switch_pgd(kinfo.global_pgd_start);
        }
    } else {
        if (pgd != next->mm.pgd)
            switch_pgd(next->mm.pgd);
    }

    return prev;
}
