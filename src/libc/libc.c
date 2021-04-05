#include <libc.h>

void __libc_init(void)
{
    __malloc_init();
}
