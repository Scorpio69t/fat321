#ifndef _KERNEL_PAGE_H_
#define _KERNEL_PAGE_H_

#include <kernel/types.h>

/* _end is the end pos of kernel.bin in the memory and defined in ld script */
extern uint64 _end;

/* use 4k page */
#define PAGE_SIZE                 0x1000
#define PRE_PAGE_ENTRY            512 /* PAGE_SIZE / 8 */
#define PAGE_TABLE_ADDRESS        (((uint64)&_end + PAGE_SIZE) & ~((uint64)PAGE_SIZE - 1))
#define PAGE_UPPER_ALIGN(address) (upper_div(address, PAGE_SIZE) * PAGE_SIZE)
#define PAGE_LOWER_ALIGN(address) (lower_div(address, PAGE_SIZE) * PAGE_SIZE)

#define PML4E_ATTR 0x007
#define PDPTE_ATTR 0x007
#define PDE_ATTR   0x007
#define PTE_ATTR   0x007

uint64        setup_page_table(uint64 memsize);
unsigned long __phy(unsigned long);
unsigned long __vir(unsigned long);

#endif
