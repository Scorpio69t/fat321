#include <kernel/bugs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/signal.h>
#include <kernel/slab.h>

unsigned long sys_getticks(void)
{
    return ticks;
}

long do_brk(unsigned long addr)
{
    proc_t *      proc = current;
    unsigned long new_brk, brk;
    int           nr_pages, i;

    new_brk = PAGE_UPPER_ALIGN(addr);
    nr_pages = (new_brk - proc->mm.brk) / PAGE_SIZE;

    for (brk = proc->mm.brk, i = 0; i < nr_pages; i++, brk += PAGE_SIZE) {
        if (map_page(proc, brk) == 0) {
            printk("pid: %d do_brk faild\n", current->pid);
            return -1;
        }
    }
    proc->mm.brk = new_brk;
    return 0;
}

void do_exit(int status)
{
    proc_t *          proc, *parent;
    struct list_head *pos, *n;
    message           mess;

    proc = current;
    parent = proc->parent;
    assert(parent != NULL);

    /* 释放用户空间内存 */
    free_proc_mm(proc);
    /* 将子进程过继给父进程 */
    list_for_each_safe(pos, n, &proc->children)
    {
        list_add(pos, &parent->children);
    }

    mess.src = proc->pid;
    mess.type = MSG_FREEFS;
    if (do_sendrecv(NULL, IPC_VFS, &mess) != 0)
        printk("free fs failed: pid %d\n", proc->pid);

    proc->exit_status = status;
    send_signal(parent, SIGCHLD);
    proc->state = PROC_STOPPED;
    schedule();
}

/* TODO: More feature */
long do_wait(int *statloc)
{
    proc_t *child, *pos;
    pid_t   pid;

    if (list_is_null(&current->children))
        return -1;
    if (current->signal != SIGCHLD) {
        current->wait = IPC_SIGNAL;
        current->state = PROC_RECEIVING;
        schedule();
    }

    child = 0;
    list_for_each_entry(pos, &current->children, child_list)
    {
        if (pos->state == PROC_STOPPED) {
            child = pos;
            break;
        }
    }
    if (statloc != NULL)
        *statloc = child->exit_status;
    pid = child->pid;

    free_page((unsigned long)child->mm.pgd);
    unmap_proc(pid);
    list_del(&child->proc);
    free_pages((unsigned long)child, KERNEL_STACK_ORDER);
    return pid;
}
