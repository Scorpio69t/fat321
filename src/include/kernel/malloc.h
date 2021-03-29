#ifndef _KERNEL_MALLOC_H_
#define _KERNEL_MALLOC_H_

#include <kernel/types.h>

void *malloc(size_t);

void free(void *);

#endif
