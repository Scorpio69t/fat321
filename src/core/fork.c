#include <alphaz/fork.h>
#include <alphaz/bugs.h>
#include <alphaz/gfp.h>
#include <alphaz/kernel.h>
#include <alphaz/linkage.h>
#include <alphaz/malloc.h>
#include <alphaz/sched.h>
#include <alphaz/type.h>
#include <alphaz/unistd.h>
#include <alphaz/mm.h>
#include <asm/process.h>
#include <asm/irq.h>

volatile pid_t global_pid = 0;

static inline pid_t alloc_pid(void)
{
    return ++global_pid;
}

static int copy_flags(struct task_struct *p, int clone_flags)
{
    p->flags = current->flags;
    return 0;
}

static int copy_fs(struct task_struct *p, int clone_flags)
{
    p->cwd = current->cwd;
    return 0;
}

static int copy_signal(struct task_struct *p, int clone_flags)
{
    p->signal = current->signal;
    return 0;
}

static unsigned long *dup_page_table(void)
{
    unsigned long *pt, *oldpt;

    oldpt = (unsigned long *)current->mm->pgd;
    pt = (unsigned long *)get_zeroed_page(GFP_KERNEL);
    if (!pt)
        return NULL;
    /* 当前只简易的拷贝一下页目录 */
    memcpy(pt, oldpt, PAGE_SIZE);
    return pt;
}

static struct mm_struct * dup_mm(void)
{
    struct mm_struct *mm, *oldmm;

    oldmm = current->mm;
    if (!oldmm)
        return NULL;

    mm = kmalloc(sizeof(struct mm_struct), 0);
    if (!mm)
        return NULL;

    memcpy(mm, oldmm, sizeof(*mm));
    mm->pgd = dup_page_table();
    if (!mm->pgd) {
        kfree(mm);
        return NULL;
    }
    return mm;
}

static int copy_mm(struct task_struct *p, int clone_flags)
{
    struct mm_struct *mm;

    /* 一些旧代码还无法废除 */
    p->stack = (void *)get_zeroed_page(GFP_USER);
    if (!p->stack)
        return -1;
    if (!(p->flags & PF_KTHREAD))
		memcpy(p->stack, current->stack, USER_STACK_SIZE);

    if (clone_flags & CLONE_VM) {
        mm = current->mm;
        goto _ret;
    }

    mm = dup_mm();

_ret:
    p->mm = mm;
    return 0;
}

static int copy_files(struct task_struct *new, int clone_flags)
{
    struct files_struct *files;

    if (clone_flags & CLONE_FS) {
        files = current->files;
        goto _ret;
    }

    files = (struct files_struct *)kmalloc(sizeof(struct files_struct), 0);
    assert(files != 0);
    if (!files)
        return -1;
    memcpy(files, current->files, sizeof(struct files_struct));

_ret:
    new->files = files;
    return 0;
}

static struct task_struct * copy_process(int clone_flags, unsigned long stack_start, struct pt_regs *regs, unsigned long stack_size)
{
	struct task_struct *p;

    p = (struct task_struct *)get_zeroed_page(GFP_KERNEL);
    if (!p)
        return 0;
    p->state = TASK_UNINTERRUPTIBLE;
    p->prio = 1;
    p->counter = 1;
    p->alarm  = 0;
    p->parent = current;

    list_head_init(&p->children);
    copy_flags(p, clone_flags);
    copy_signal(p, clone_flags);
    copy_fs(p, clone_flags);

    if(copy_files(p, clone_flags))
        goto copy_failed;
    if (copy_mm(p, clone_flags))
        goto copy_failed;

    setup_thread(p, regs, clone_flags);

    return p;
copy_failed:
    free_page((unsigned long)p);
    return 0;
}

long do_fork(int clone_flags, unsigned long stack_start, struct pt_regs *regs,
			unsigned long stack_size)
{
	struct task_struct *p;

	p = copy_process(clone_flags, stack_start, regs, stack_size);
    if (!p)
        return -1;

    p->pid = alloc_pid();

    cli();
    list_add(&p->sibling, &current->children);
	list_add_tail(&p->task, &task_head);
    sti();
	p->state = TASK_RUNNING;
    return p->pid;
}

asmlinkage int sys_fork(void)
{
	struct pt_regs *regs = (struct pt_regs *)current->thread.esp0 - 1;
	return do_fork(0, 0, regs, 0);
}

int kernel_thread(int (*fn)(void), void *args, unsigned long flags)
{
	struct pt_regs regs;
	return _kernel_thread(&regs, fn, args, flags);
}
