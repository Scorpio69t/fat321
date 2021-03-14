#ifndef _FENG_MM_TYPES_H_
#define _FENG_MM_TYPES_H_

#include <boot/atomic.h>
#include <feng/list.h>
#include <feng/slab.h>

struct minfo {
    unsigned int base_addr_low;
    unsigned int base_addr_high;
    unsigned int length_low;
    unsigned int length_high;
    unsigned int type;
} __attribute__((packed));

struct page {
    unsigned long      flags;
    atomic_t           _count; /* 使用计数 */
    struct list_head   list;   /* 页块列表 */
    struct kmem_cache *slab;   /* slab使用 */
    void *virtual;             /* 内核虚拟地址，为NULL为高端内存 */
};

#endif
