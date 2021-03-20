#include <boot/memory.h>
#include <feng/mm.h>
#include <feng/types.h>

/**
 * get_pgd - 获取页目录的起始逻辑地址
 */
uint64 get_pgd(void)
{
    unsigned long pgd;
    asm volatile("movq %%cr3, %0" : "=r"(pgd)::"memory");
    return to_vir(pgd);
}

/**
 * switch_pgd - 切换页表
 * @pgd: 页目录的起始逻辑地址
 */
void switch_pgd(uint64 pgd)
{
    asm volatile(
        "movq %0, %%cr3\n\t"
        "jmp 1f\n\t"
        "1:\t" ::"r"(to_phy(pgd))
        : "memory");
}

void flash_tlb(void)
{
    // asm volatile(
    // "movl %%cr3, %%eax\n\t"
    // "movl %%eax, %%cr3\n\t"
    // "jmp 1f\n\t"
    // "1f: \t" ::
    //     : "eax", "memory");
}