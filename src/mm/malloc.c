#include <feng/malloc.h>
#include <feng/slab.h>
#include <feng/types.h>

void *malloc(size_t size)
{
    return kmalloc(size, 0);
}

void free(void *addr)
{
    kfree(addr);
}
