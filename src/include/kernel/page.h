#ifndef _KERNEL_PAGE_H_
#define _KERNEL_PAGE_H_

#include <kernel/sched.h>
#include <kernel/types.h>

/* use 4k page */
#define PAGE_SIZE                 0x1000
#define PRE_PAGE_ENTRY            512 /* PAGE_SIZE / 8 */
#define PAGE_UPPER_ALIGN(address) (upper_div(address, PAGE_SIZE) * PAGE_SIZE)
#define PAGE_LOWER_ALIGN(address) (lower_div(address, PAGE_SIZE) * PAGE_SIZE)

#define PML4E_ATTR 0x007
#define PDPTE_ATTR 0x007
#define PDE_ATTR   0x007
#define PTE_ATTR   0x007

uint64 setup_page_table(uint64 memsize);
uint64 map_page(proc_t *proc, uint64 ustart);
uint64 unmap_page(proc_t *proc, uint64 ustart);
void *kmap(void *uaddr);
int free_proc_mm(proc_t *proc);

#endif
