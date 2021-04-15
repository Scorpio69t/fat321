#include <boot/irq.h>
#include <boot/process.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/types.h>

volatile pid_t global_pid = 128;

static inline pid_t alloc_pid(void)
{
    return ++global_pid;
}

static int copy_mm(proc_t *proc)
{
    proc_t *      cur;
    int           i;
    unsigned long vstart, vend;
    unsigned long start, end;
    unsigned long kstart;

    cur = current;
    proc->mm.flags = cur->mm.flags;

    /* copy level4 page table */
    proc->mm.pgd = get_zeroed_page(GFP_KERNEL);
    if (!proc->mm.pgd)
        return -1;

    memcpy((void *)proc->mm.pgd, (void *)cur->mm.pgd, PAGE_SIZE);
    memset((void *)proc->mm.pgd, 0x00, PAGE_SIZE / 2);

    /* copy page segment */
    proc->mm.nr_seg = cur->mm.nr_seg;
    memcpy((void *)proc->mm.psegs, (void *)cur->mm.psegs, sizeof(proc->mm.psegs));

    for (i = 0; i < cur->mm.nr_seg; i++) {
        vstart = cur->mm.psegs[i].vstart;
        vend = cur->mm.psegs[i].vend;

        start = PAGE_LOWER_ALIGN(vstart);
        end = PAGE_UPPER_ALIGN(vend);
        while (start != end) {
            kstart = map_page(proc, start);
            memcpy((void *)kstart, (void *)start, PAGE_SIZE);
            start += PAGE_SIZE;
        }
    }

    /* copy user stack */
    start = proc->mm.start_stack = cur->mm.start_stack;
    end = proc->mm.end_stack = cur->mm.end_stack;
    while (start != end) {
        kstart = map_page(proc, start);
        memcpy((void *)kstart, (void *)start, PAGE_SIZE);
        start += PAGE_SIZE;
    }

    /* copy heap */
    start = proc->mm.start_brk = cur->mm.start_brk;
    end = proc->mm.brk = cur->mm.brk;
    while (start != end) {
        kstart = map_page(proc, start);
        memcpy((void *)kstart, (void *)start, PAGE_SIZE);
        start += PAGE_SIZE;
    }
    return 0;
}

static int copy_fs(frame_t *regs, proc_t *proc)
{
    message mess;

    assert(proc->pid != 0);

    mess.type = MSG_COPYFS;
    mess.src = current->pid;
    mess.m_copyfs.pid = proc->pid;
    if (do_sendrecv(regs, IPC_VFS, &mess) != 0 || mess.retval != 0)
        return -1;
    return 0;
}

long do_fork(frame_t *regs)
{
    proc_t *proc;

    proc = (proc_t *)__get_free_pages(GFP_KERNEL, KERNEL_STACK_ORDER);
    if (!proc)
        return NULL;
    memset(proc, 0x00, PAGE_SIZE * (1 << KERNEL_STACK_ORDER));
    proc->state = PROC_SENDING;
    proc->pid = alloc_pid();
    proc->counter = 1;
    proc->alarm = 0;
    proc->parent = current;
    list_head_init(&proc->wait_proc);

    if (copy_mm(proc))
        goto faild;
    if (copy_fs(regs, proc))
        goto faild;

    copy_context(proc, regs);
    disable_interrupt();
    list_add_tail(&proc->proc, &scheduler.proc_head);
    hash_proc(proc);
    enable_interrupt();
    proc->state = PROC_RUNNABLE;
    return proc->pid;

faild:
    /*TODO: free_mm*/
    free_pages((unsigned long)proc, KERNEL_STACK_ORDER);
    return -1;
}
