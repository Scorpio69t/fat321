#include <boot/irq.h>
#include <boot/process.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/malloc.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/types.h>
#include <kernel/unistd.h>

volatile pid_t global_pid = 0;

static inline pid_t alloc_pid(void)
{
    return ++global_pid;
}

static int copy_flags(struct proc_struct *p, int clone_flags)
{
    p->flags = current->flags;
    return 0;
}

// static int copy_fs(struct proc_struct *p, int clone_flags)
// {
//     p->cwd = current->cwd;
//     return 0;
// }

static int copy_signal(struct proc_struct *p, int clone_flags)
{
    p->signal = current->signal;
    return 0;
}

static int copy_mm(struct proc_struct *p, int clone_flags)
{
    /* 一些旧代码还无法废除 */
    p->stack = (void *)__get_free_pages(GFP_USER, USER_STACK_ORDER);
    if (!p->stack)
        return -1;
    if (!(p->flags & PF_KTHREAD))
        memcpy(p->stack, current->stack, USER_STACK_SIZE);

    if (clone_flags & CLONE_VM) {
        p->mm = current->mm;
        goto _ret;
    }

_ret:
    return 0;
}

// static int copy_files(struct proc_struct *new, int clone_flags)
// {
//     struct files_struct *files;

//     if (clone_flags & CLONE_FS) {
//         files = current->files;
//         goto _ret;
//     }

//     files = (struct files_struct *)kmalloc(sizeof(struct files_struct), 0);
//     assert(files != 0);
//     if (!files)
//         return -1;
//     memcpy(files, current->files, sizeof(struct files_struct));

// _ret:
//     new->files = files;
//     return 0;
// }

static struct proc_struct *copy_process(int clone_flags, unsigned long stack_start, struct pt_regs *regs,
                                        unsigned long stack_size)
{
    struct proc_struct *p;
    struct pt_regs *    childregs;

    p = (struct proc_struct *)__get_free_pages(GFP_KERNEL, KERNEL_STACK_ORDER);
    if (!p)
        return NULL;
    p->state = TASK_SENDING;
    p->counter = 1;
    p->alarm = 0;
    p->parent = current;

    copy_flags(p, clone_flags);
    copy_signal(p, clone_flags);
    // copy_fs(p, clone_flags);

    // if (copy_files(p, clone_flags))
    //     goto copy_failed;
    if (copy_mm(p, clone_flags))
        goto copy_failed;

    copy_context(p, regs, clone_flags);

    return p;
copy_failed:
    free_page((unsigned long)p);
    return 0;
}

long do_fork(int clone_flags, unsigned long stack_start, struct pt_regs *regs, unsigned long stack_size)
{
    struct proc_struct *p;

    p = copy_process(clone_flags, stack_start, regs, stack_size);
    if (!p)
        return -1;

    p->pid = alloc_pid();

    disable_interrupt();
    list_add_tail(&p->proc, &scheduler.proc_head);
    enable_interrupt();
    p->state = TASK_RUNNABLE;
    return p->pid;
}

int sys_fork(void)
{
    struct pt_regs *regs = (struct pt_regs *)current->context.rsp0 - 1;
    return do_fork(0, 0, regs, 0);
}
