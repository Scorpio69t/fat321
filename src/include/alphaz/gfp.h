#ifndef _ALPAHZ_GFP_H_
#define _ALPAHZ_GFP_H_

#include <alphaz/mm_types.h>

void buddy_system_init(void);

struct page * alloc_pages(unsigned int gfp_mask, unsigned int order);
struct page * alloc_page(unsigned int gfp_mask);
unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);
unsigned long __get_free_page(unsigned int gfp_mask);
unsigned long get_zeroed_page(unsigned int gfp_mask);

void __free_pages(struct page *page, unsigned int order);
void free_pages(unsigned long addr, unsigned int order);
void free_page(unsigned long addr);

#define GFP_KERNEL      (1UL << 0)          /* 分配内核区的页 */
#define GFP_USER        (1UL << 1)

#endif
