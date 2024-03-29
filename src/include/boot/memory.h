#ifndef _BOOT_MEMORY_H_
#define _BOOT_MEMORY_H_

#include <kernel/mm.h>
#include <kernel/page.h>

uint64 get_pgd(void);
void switch_pgd(uint64 pgd);
void flash_tlb(void);

#endif
