#ifndef  _ALPHAZ_STRING_H_
#define _ALPHAZ_STRING_H_

#include <asm/string.h>

#ifndef __HAVE_ARCH_MEMCPY
extern void * memcpy(void *, void *, size_t);
#endif

#ifndef __HAVE_ARCH_MEMSET
extern void * memset(void *, u8, size_t);
#endif

#ifndef __HAVE_ARCH_STRCPY
extern void * strcpy(char *dest, const char *src);
#endif

#ifndef __HAVE_ARCH_STRNCPY
extern void * strncpy(char *dest, const char *src, size_t n);
#endif

#ifndef __HAVE_ARCH_STRLEN
extern size_t strlen(const char *s);
#endif

#ifndef __HAVA_ARCH_STRCMP
extern int strcmp(const char *str1, const char *str2);
#endif

#ifndef __HAVE_ARCH_STRNCMP
extern int strncmp(const char *str1, const char *str2, size_t n);
#endif

#endif
