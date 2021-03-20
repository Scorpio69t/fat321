#ifndef _FENG_MM_H_
#define _FENG_MM_H_

#include <boot/boot.h>

#define __KERNEL_OFFSET KERNEL_OFFSET
#define __USER_OFFSET   0x40000000

#ifndef __ASSEMBLY__
#include <boot/atomic.h>
#include <feng/list.h>
#include <feng/types.h>

extern uint64 _start;
#define KERNEL_START ((uint64)&_start)

struct page {
    struct list_head   list; /* 页块列表 */
    unsigned long      flags;
    atomic_t           _count; /* 使用计数 */
    struct kmem_cache *slab;   /* slab使用 */
    void *virtual;             /* 内核虚拟地址 */
};

extern struct page *mem_map;

/* 页属性 */
#define PF_RESERVE (1UL << 0) /* 保留页，操作系统不能使用 */
#define PF_BUSY    (1UL << 1)

void mm_init();

#define to_phy(address)        ((uint64)address - KERNEL_OFFSET)
#define to_vir(address)        ((uint64)address + KERNEL_OFFSET)
#define phy_ptr(type, address) ((type *)to_phy(address))
#define vir_ptr(type, address) ((type *)to_vir(address))

#endif

#endif
