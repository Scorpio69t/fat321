#ifndef _KERNEL_STRING_H_
#define _KERNEL_STRING_H_

#include <boot/string.h>

#ifndef __HAVE_ARCH_MEMCPY
extern void *memcpy(void *, void *, size_t);
#endif

#ifndef __HAVE_ARCH_MEMSET
extern void *memset(void *, u8, size_t);
#endif

#ifndef __HAVE_ARCH_STRCPY
extern void *strcpy(char *dest, const char *src);
#endif

static inline void *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && *src != 0; i++, src++) dest[i] = *src;
    return dest;
}

#ifndef __HAVE_ARCH_STRLEN
extern size_t strlen(const char *s);
#endif

#ifndef __HAVA_ARCH_STRCMP
extern int strcmp(const char *str1, const char *str2);
#endif

static inline int strncmp(const char *str1, const char *str2, size_t n)
{
    size_t i;
    int    retval;

    for (i = 0; i < n; i++, str1++, str2++) {
        retval = *str1 - *str2;
        if (retval)
            break;
    }
    return retval;
}

#endif
