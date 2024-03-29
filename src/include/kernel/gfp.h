#ifndef _ALPAHZ_GFP_H_
#define _ALPAHZ_GFP_H_

#include <kernel/mm.h>
#include <kernel/spinlock.h>

/* 伙伴系统的最长连续的页数 2^(MAX_ORDER - 1) */
#define MAX_ORDER 8

struct buddy_struct {
    struct list_head block[MAX_ORDER];
    struct list_head activate;
    spinlock_t lock;
};

void buddy_system_init(uint32 nr_pages);

struct page *alloc_pages(unsigned int gfp_mask, unsigned int order);
struct page *alloc_page(unsigned int gfp_mask);
unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);
unsigned long __get_free_page(unsigned int gfp_mask);
unsigned long get_zeroed_page(unsigned int gfp_mask);

void __free_pages(struct page *page, unsigned int order);
void free_pages(unsigned long addr, unsigned int order);
void free_page(unsigned long addr);

#define GFP_KERNEL (1UL << 0) /* 分配内核区的页 */
#define GFP_USER   (1UL << 1)

#endif
