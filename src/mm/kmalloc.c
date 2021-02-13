#include <alphaz/slab.h>
#include <alphaz/type.h>
#include <alphaz/mm.h>
#include <alphaz/gfp.h>
#include <alphaz/bugs.h>

#define KMALLOC_CACHE_SIZE  11

struct kmalloc_cache {
    struct kmem_cache *cachep;
    const unsigned int size;
};

static struct kmalloc_cache kmalloc_cache[KMALLOC_CACHE_SIZE] = {
    [0] = { .size = (1 << 5), .cachep = NULL, },    /* 32 */
    [1] = { .size = (1 << 6), .cachep = NULL, },
    [2] = { .size = (1 << 7), .cachep = NULL, },
    [3] = { .size = (1 << 8), .cachep = NULL, },
    [4] = { .size = (1 << 9), .cachep = NULL, },
    [5] = { .size = (1 << 10), .cachep = NULL, },
    [6] = { .size = (1 << 11), .cachep = NULL, },
    [7] = { .size = (1 << 12), .cachep = NULL, },
    [8] = { .size = (1 << 13), .cachep = NULL, },
    [9] = { .size = (1 << 14), .cachep = NULL, },
    [10] = { .size = (1 << 15), .cachep = NULL, },  /* 32k */
};

void * kmalloc(size_t size, int mask)
{
    int i = 0;

    while (i < KMALLOC_CACHE_SIZE && size > kmalloc_cache[i].size)
        i++;
    if (i >= KMALLOC_CACHE_SIZE)
        return NULL;
    return kmem_cache_alloc(kmalloc_cache[i].cachep, GFP_KERNEL);
}

void kfree(void *addr)
{
    unsigned long phy, ind;

    phy = (unsigned long)vir_to_phy(addr);
    ind = phy / PAGE_SIZE;
    kmem_cache_free(mem_map[ind].slab, addr);
}

void kmalloc_cache_init(void)
{
    int i;
    struct kmem_cache *cachep;

    for (i = 0; i < KMALLOC_CACHE_SIZE; ++i) {
        cachep = kmem_cache_create(NULL, kmalloc_cache[i].size, 0);
        if (!cachep)
            panic("kmalloc cache init error %d\n", kmalloc_cache[i].size);
        kmalloc_cache[i].cachep = cachep;
    }
}
