#ifndef _ALPHZ_MMZONE_H_
#define _ALPHZ_MMZONE_H_

#include <alphaz/list.h>
#include <alphaz/spinlock.h>

enum zone_type {
    ZONE_KERNEL,
    ZONE_USER,
    MAX_NR_ZONES,
};

/* 各个类型的zone的起始与结束地址（物理地址） */
#define ZONE_KERNEL_BEGIN   0x00000000
#define ZONE_KERNEL_END     0x40000000
#define ZONE_USER_BEGIN     0x40000000
#define ZONE_USER_END       0xffffffff

/* 伙伴系统的最长连续的页数 2^(MAX_ORDER - 1) */
#define MAX_ORDER   8

struct free_area {
    struct list_head free_list;         /* 页块链表 */
    unsigned long nr_free;              /* 空闲页块的数目，注意，这里将一组连续的物理页看做一个页块
                                            所以一个页块可能包含一个页，也可能包含两个，或四个等等 */
};

struct zone {
    unsigned long flags;
    spinlock_t lock;
    struct free_area free_area[MAX_ORDER];
    struct page *first_page;            /* 起始页 */
    unsigned long nr_pages;             /* 页数量 */
    struct list_head activate;          /* 已使用的页块，供伙伴系统回收时使用 */
    const char *name;
};

#endif
