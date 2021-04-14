#include <kernel/bugs.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/slab.h>

unsigned long sys_getticks(void)
{
    return ticks;
}

long do_brk(unsigned long addr)
{
    proc_t*       proc = current;
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
