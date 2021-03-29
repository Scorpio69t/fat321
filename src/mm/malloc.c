#include <kernel/malloc.h>
#include <kernel/slab.h>
#include <kernel/types.h>

void *malloc(size_t size)
{
    return kmalloc(size, 0);
}

void free(void *addr)
{
    kfree(addr);
}
