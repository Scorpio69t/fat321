#include <alphaz/type.h>
#include <alphaz/slab.h>
#include <alphaz/malloc.h>

void * malloc(size_t size)
{
    return kmalloc(size, 0);
}

void free(void *addr)
{
    kfree(addr);
}
