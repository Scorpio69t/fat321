#ifndef _ALPHAZ_SLAB_H_
#define _ALPHAZ_SLAB_H_

#include <alphaz/type.h>
#include <alphaz/list.h>

#define SLAB_MIN_NUM     4      /* 计算时每个slab最少的对象数，最后实际上可能比这少一个 */
#define SLAB_MAX_NUM     256    /* 计算时每个slab最多的对象数 */
#define SLAB_MAX_FREE    4      /* 最多允许的空闲slab */
#define BUFCTL_END       0xffff /* slab管理数组的结束标记 */

struct slab {
    struct list_head  list;
    void              *s_mem;   /* slab中的第一个对象   */
    unsigned int      inuse;    /* slab中已分配的对象数 */
    unsigned int      free;     /* 第一个空闲对象所在的位置 */
};

struct kmem_cache {
    unsigned int      flags;
    unsigned int      total_slab;       /* slab总数 */
    unsigned int      free_slab;        /* 空闲的slab数 */
    unsigned int      num;              /* 每个slab中对象的数量 */
    unsigned int      size;             /* 每个对象的大小，包含填充字节 */

    unsigned int      gfporder;         /* 每个slab的页数，取以2为底的对数 */
    unsigned int      gfpflags;         /* 页分配时GFP标志 */

    struct list_head  slabs_partial;    /* 部分空闲的slab链表 */
    struct list_head  slabs_full;       /* 已满的slab链表 */
    struct list_head  slabs_free;       /* 空的slab链表 */
};

void * kmalloc(size_t, int);
void kfree(void *);
void kmalloc_cache_init(void);

void kmem_cache_init(void);
struct kmem_cache * kmem_cache_create(const char *, size_t, unsigned int);
int kmem_cache_destroy(struct kmem_cache *);
void * kmem_cache_alloc(struct kmem_cache *, unsigned int);
void kmem_cache_free(struct kmem_cache *, void *);

#endif
