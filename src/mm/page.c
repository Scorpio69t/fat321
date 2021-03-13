#include <boot/bug.h>
#include <boot/memory.h>
#include <boot/sched.h>
#include <feng/gfp.h>
#include <feng/kernel.h>
#include <feng/mm.h>
#include <feng/page.h>
#include <feng/slab.h>
#include <feng/string.h>

unsigned long __phy(unsigned long addr)
{
    if (addr >= __KERNEL_OFFSET)
        return addr - __KERNEL_OFFSET;
    return addr + __USER_OFFSET;
}

unsigned long __vir(unsigned long addr)
{
    if (addr <= __USER_OFFSET)
        return addr + __KERNEL_OFFSET;
    return addr - __USER_OFFSET;
}

static void fill_pde(unsigned long pde, unsigned long pte, unsigned long nr)
{
    int            i;
    unsigned long *pd, phyaddr;
    unsigned long  ki, ui;  // kernel index, user index

    phyaddr = __phy(pte);
    nr = nr / NUM_PER_PAGE + (nr % NUM_PER_PAGE ? 1 : 0);

    pd = (unsigned long *)pde;
    ki = KERNEL_BASE / (PAGE_SIZE * NUM_PER_PAGE);  // 内核页目录起始位置
    ui = USER_BASE / (PAGE_SIZE * NUM_PER_PAGE);    // 用户页目录起始地址

    for (i = ki; i < NUM_PER_PAGE && nr; i++, nr--, phyaddr += PAGE_SIZE) pd[i] = phyaddr | PAGE_ATTR;

    for (i = ui; i < ki && nr; i++, nr--, phyaddr += PAGE_SIZE) pd[i] = phyaddr | PAGE_ATTR;
}

static void fill_pte(unsigned long pte, unsigned long nr)
{
    int            i;
    unsigned long *pt, phyaddr;

    pt = (unsigned long *)pte;
    phyaddr = 0x00;

    for (i = 0; i < nr; i++, phyaddr += PAGE_SIZE) pt[i] = phyaddr | PAGE_ATTR;
}

unsigned long reset_page_table(unsigned long memsize)
{
    unsigned long pde, pte, addr;
    unsigned long nr;

    addr = PAGE_PDE;                                          // 获取到我们要保存页表的地址
    pde = (addr / PAGE_SIZE + 1) * PAGE_SIZE;                 // 页目录起始地址，4k对其
    pte = pde + PAGE_SIZE;                                    // 页表起始地址
    nr = memsize / PAGE_SIZE + (memsize % PAGE_SIZE ? 1 : 0); /* 页表项数 */

    memset((void *)pde, 0, PAGE_SIZE);

    fill_pte(pte, nr);
    fill_pde(pde, pte, nr);
    switch_pgd(pde);
    return pte + nr * sizeof(unsigned long);  // 返回整个页表结构占用内存的尾地址
}
