/*
 * slab高速缓存
 */

#include <alphaz/type.h>
#include <alphaz/slab.h>
#include <alphaz/gfp.h>
#include <alphaz/bugs.h>
#include <alphaz/mm.h>
#include <alphaz/kernel.h>

static inline unsigned int kmem_align(unsigned int size)
{
    unsigned int ret;
    ret = size / sizeof(void *) * sizeof(void *);
    ret += size % sizeof(void *) ? sizeof(void *) : 0;
    return ret;
}

static inline void * kmem_getpages(struct kmem_cache *cachep, unsigned int flags)
{
    int i;
    struct page *page;

    flags |= cachep->gfpflags;
    page = alloc_pages(flags, cachep->gfporder);

    if (!page)
        return 0;

    for (i = 0; i < (1 << cachep->gfporder); i++)
        page[i].slab = cachep;

    return page->virtual;
}

static int cache_grow(struct kmem_cache *cachep)
{
    unsigned int head, i;
    unsigned short *array;
    struct slab *slabp;

    slabp = (struct slab *)kmem_getpages(cachep, 0);
    if (!slabp) return -1;

    slabp->inuse = 0;
    slabp->free = 0;
    head = sizeof(struct slab) + (cachep->num + 1) * sizeof(unsigned short);
    head = kmem_align(head);
    slabp->s_mem = (void *)slabp + head;

    array = (unsigned short *)((void *)slabp + sizeof(struct slab));
    for (i = 0; i < cachep->num; i++)
        array[i] = i;
    array[cachep->num] = BUFCTL_END;

    cachep->total_slab++;
    cachep->free_slab++;
    list_add(&slabp->list, &cachep->slabs_free);

    return 0;
}

/**
 * kmem_cache_create - 创建高速缓存
 * @name: 缓存名称
 * @size: 每个元素的大小
 * @flags: 可选的设置项
 *
 * 当前的高速缓冲分配方式非常简单，我们的kmem_cache结构体会单独占用一个页，每个slab的头部信
 * 息都会放在一个每个slab里，不使用外部的空间
 */
struct kmem_cache * kmem_cache_create(const char *name, size_t size, unsigned int flags)
{
    unsigned int tnum, head, i;
    struct kmem_cache *cachep = NULL;

    /* 如果对象的大小超过了可连续分配的页的大小的四分之一，则不使用缓存 */
    if (size >= PAGE_SIZE * (1 << (MAX_ORDER - 3)))
        return 0;

    cachep = (struct kmem_cache *)__get_free_page(GFP_KERNEL);
    if (!cachep)
        return 0;
    cachep->flags = flags;
    cachep->gfpflags = GFP_KERNEL;
    cachep->free_slab = 0;
    cachep->total_slab = 0;

    list_head_init(&cachep->slabs_partial);
    list_head_init(&cachep->slabs_full);
    list_head_init(&cachep->slabs_free);

    cachep->size = kmem_align(size);

    /* 计算每个slab至少需要几个页 */
    i = 0;
    while (cachep->size * SLAB_MIN_NUM > PAGE_SIZE * (1 << i) && i < MAX_ORDER)
        i++;
    if (i >= MAX_ORDER) {    /* 对象太大，无法获取足够的空间 */
        free_page((unsigned long)cachep);
        return 0;
    }
    cachep->gfporder = i;

    /* 计算slab中对象的个数 */
    tnum = (1 << cachep->gfporder) * PAGE_SIZE / cachep->size;
    head = sizeof(struct slab) + (tnum + 1) * sizeof(unsigned short);  // 包含一个结束标记
    head = kmem_align(head);
    cachep->num = ((1 << cachep->gfporder) * PAGE_SIZE - head) / cachep->size;

    if(cache_grow(cachep)) {
        free_page((unsigned long)cachep);
        return 0;
    }
    return cachep;
}

/**
 * kmem_cache_destroy - 销毁高速缓存
 * @cachep: 高速缓存指针
 */
int kmem_cache_destroy(struct kmem_cache *cachep)
{
    struct list_head *pos, *n;
    struct slab *slabp;

    list_for_each_safe(pos, n, &cachep->slabs_partial) {
        slabp = list_entry(pos, struct slab, list);
        free_pages((unsigned long)slabp, cachep->gfporder);
    }

    list_for_each_safe(pos, n, &cachep->slabs_full) {
        slabp = list_entry(pos, struct slab, list);
        free_pages((unsigned long)slabp, cachep->gfporder);
    }

    list_for_each_safe(pos, n, &cachep->slabs_free) {
        slabp = list_entry(pos, struct slab, list);
        free_pages((unsigned long)slabp, cachep->gfporder);
    }

    free_page((unsigned long)cachep);
    return 0;
}

static void * ____cache_alloc(struct kmem_cache *cachep, struct slab *slabp)
{
    void *ret = NULL;
    unsigned short *array;

    array = (unsigned short *)((void *)slabp + sizeof(struct slab));

    if (array[slabp->free] != BUFCTL_END) {
        ret = slabp->s_mem + cachep->size * array[slabp->free];
        slabp->free++;
        slabp->inuse++;
    }

    if (array[slabp->free] == BUFCTL_END) {     // 当前slab已满
        list_del(&slabp->list);
        list_add(&slabp->list, &cachep->slabs_full);
    }
    return ret;
}

static void * __cache_alloc(struct kmem_cache *cachep)
{
    struct list_head *pos, *n;
    struct slab *slabp;
    void *ret = NULL;
    int infree = 0;

    list_for_each_safe(pos, n, &cachep->slabs_partial) {
        slabp = list_entry(pos, struct slab, list);
        ret = ____cache_alloc(cachep, slabp);
        if (ret) goto _ret;
    }

    list_for_each_safe(pos, n, &cachep->slabs_free) {
        slabp = list_entry(pos, struct slab, list);
        ret = ____cache_alloc(cachep, slabp);
        infree = 1;
        if (ret) goto _ret;
    }

    return 0;
_ret:
    if (infree) {
        list_del(&slabp->list);
        list_add(&slabp->list, &cachep->slabs_partial);
        cachep->free_slab--;
    }
    return ret;
}

/**
 * kmem_cache_alloc - 从高速缓存中分配对象
 * @cachep: 高速缓存指针
 * @flags: 分配时的标示，传送给__get_free_pages
 */
void * kmem_cache_alloc(struct kmem_cache *cachep, unsigned int flags)
{
    void *ret;

    ret = __cache_alloc(cachep);
    if (!ret && !cache_grow(cachep))
        ret = __cache_alloc(cachep);
    return ret;
}

/* 计算对象地址在slab中的下标，若该对象不在本slab中，返回-1 */
static inline int slab_index(struct kmem_cache *cachep, struct slab *slabp, void *objp)
{
    if (objp < slabp->s_mem)
        return -1;
    if (objp > slabp->s_mem + (cachep->num - 1) * cachep->size)
        return -1;
    return (unsigned long)(objp - slabp->s_mem) / cachep->size;
}

static void __cache_free(struct kmem_cache *cachep, void *objp)
{
    int ind;
    unsigned short *array;
    struct list_head *pos, *n;
    struct slab *slabp = NULL;

    list_for_each_safe(pos, n, &cachep->slabs_full) {
        slabp = list_entry(pos, struct slab, list);
        ind = slab_index(cachep, slabp, objp);
        if (ind != -1)
            goto _free;
    }

    list_for_each_safe(pos, n, &cachep->slabs_partial) {
        slabp = list_entry(pos, struct slab, list);
        ind = slab_index(cachep, slabp, objp);
        if (ind != -1)
            goto _free;
    }

_free:
    array = (unsigned short *)((void *)slabp + sizeof(struct slab));
    if (slabp->free) {          // slabp->free不能为0，否则会溢出slab的管理数组
        array[--slabp->free] = ind;
        slabp->inuse--;
    }
    if (!slabp->inuse) {
        list_del(&slabp->list);
        list_add(&slabp->list, &cachep->slabs_free);
        cachep->free_slab++;
    }
}

/**
 * kmem_cache_free - 从高速缓冲中释放对象
 * @cachep: 高速缓存指针
 * @objp: 对象指针
 */
void kmem_cache_free(struct kmem_cache *cachep, void *objp)
{
    struct slab *slabp;

    __cache_free(cachep, objp);
    while (cachep->free_slab > SLAB_MAX_FREE) {
        slabp = list_first_entry(&cachep->slabs_free, struct slab, list);
        free_pages((unsigned long)slabp, cachep->gfporder);
        cachep->free_slab--;
    }
}

void kmem_cache_init(void)
{
    void *p;
    struct kmem_cache *cachep = kmem_cache_create("test", 500, 0);
    printk("++++++++++++++++\n");
    if (!cachep)
        printk("error\n");
    else
        printk("%d %d %d\n", cachep->size, cachep->num, cachep->gfporder);

    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);
    kmem_cache_free(cachep, p);
    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);

    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);
    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);
    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);
    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);
    p = kmem_cache_alloc(cachep, 0);
    printk("%p\n", p);

    kmem_cache_destroy(cachep);
}
