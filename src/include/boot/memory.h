#ifndef _ASM_MEMORY_H_
#define _ASM_MEMORY_H_

#include <alphaz/page.h>

/**
 * get_pgd - 获取页目录的起始逻辑地址
 */
static inline unsigned long get_pgd(void)
{
    unsigned long pgd;
    asm volatile("movl %%cr3, %0":"=r"(pgd)::"memory");
    return __vir(pgd);
}

/**
 * switch_pgd - 切换页表
 * @pgd: 页目录的起始逻辑地址
 */
static inline void switch_pgd(unsigned long pgd)
{
    asm volatile(
        "mov %0, %%cr3\n\t"
        "jmp 1f\n\t"
        "1:\t"
        ::"r"(__phy(pgd)):"memory");
}

static inline void flash_tlb(void)
{
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %%cr3\n\t"
        "jmp 1f\n\t"
        "1f: \t"
        :::"eax", "memory");
}

#endif
