#ifndef _ALPHAZ_PAGE_H_
#define _ALPHAZ_PAGE_H_

#include <feng/mm_types.h>

extern unsigned long _end;
#define PAGE_ATTR    0x00000007
#define TOTAL_PDE    1024 /* 页目录数量 */
#define NUM_PER_PAGE 1024 /* 每页的目录条目数 */
#define PAGE_PDE     ((unsigned long)&_end)

unsigned long reset_page_table(unsigned long memsize);
unsigned long __phy(unsigned long);
unsigned long __vir(unsigned long);
#endif
