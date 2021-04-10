#ifndef _BOOT_STRING_H_
#define _BOOT_STRING_H_

#include <kernel/types.h>

#define __HAVE_ARCH_MEMCPY
static inline void *memcpy(void *to, void *from, size_t n)
{
    uint64 d0, d1, d2;
    asm volatile(
        "nop\n\t"
        "nop\n\t"
        "rep ; movsb\n\t"
        : "=&c"(d0), "=&D"(d1), "=&S"(d2)
        : "0"(n), "1"((uint64)to), "2"((uint64)from)
        : "memory");

    return to;
}

#define __HAVE_ARCH_MEMSET
static inline void *memset(void *from, u8 value, size_t n)
{
    uint64 d0, d1;
    u8     d2;
    asm volatile(
        "1: movb %%al, (%%rdi)\n\t"
        "dec %%rcx\n\t"
        "inc %%rdi\n\t"
        "testq %%rcx, %%rcx\n\t"
        "jnz 1b\n\t"
        "2:"
        : "=&c"(d0), "=&D"(d1), "=&a"(d2)
        : "0"(n), "1"((uint64)from), "2"(value)
        : "memory");

    return from;
}

#define __HAVE_ARCH_STRCPY
static inline void *strcpy(char *dest, const char *src)
{
    int d0, d1;
    asm volatile(
        "1:"
        "movb (%%rsi), %%al\n\t"
        "movb %%al, (%%rdi)\n\t"
        "inc %%rsi\n\t"
        "inc %%rdi\n\t"
        "cmp $0, %%al\n\t"
        "jnz 1b\n\t"
        : "=&D"(d0), "=&S"(d1)
        : "0"((unsigned long)dest), "1"((unsigned long)src)
        : "%eax", "memory");

    return dest;
}

#define __HAVE_ARCH_STRNCPY
static inline void *strncpy(char *dest, const char *src, size_t n)
{
    int d0, d1, d2;
    asm volatile(
        "1:\t dec %%ecx\n\t"
        "js 2f\n\t"
        "movb (%%esi), %%al\n\t"
        "movb %%al, (%%edi)\n\t"
        "inc %%esi\n\t"
        "inc %%edi\n\t"
        "cmp $0, %%al\n\t"
        "jnz 1b\n\t"
        "2:\t"
        : "=&D"(d0), "=&S"(d1), "=&c"(d2)
        : "0"(dest), "1"(src), "2"(n)
        : "%eax", "memory");

    return dest;
}

#define __HAVE_ARCH_STRLEN
static inline size_t strlen(const char *s)
{
    int d0, d1;
    asm volatile(
        "xor %%rcx, %%rcx\n\t"
        "1:\tcmpb $0, (%%rsi)\n\t"
        "jz 2f\n\t"
        "inc %%rcx\n\t"
        "inc %%rsi\n\t"
        "jmp 1b\n\t"
        "2:"
        : "=c"(d0), "=&S"(d1)
        : "1"((unsigned long)s));
    return (size_t)d0;
}

#define __HAVA_ARCH_STRCMP
static inline int strcmp(const char *str1, const char *str2)
{
    int d0, d1, d2;
    asm volatile(
        "0:\t lodsb\n\t"
        "scasb\n\t"
        "jne 1f\n\t"
        "testb %%al, %%al\n\t"
        "jne 0b\n\t"
        "xorl %%eax, %%eax\n\t"
        "jmp 2f\n\t"
        "1:\t sbbl %%eax, %%eax\n\t"
        "orb $1, %%al\n\t"
        "2:\t"
        : "=a"(d0), "=&S"(d1), "=&D"(d2)
        : "1"(str1), "2"(str2)
        : "memory");
    return d0;
}

#define __HAVE_ARCH_STRNCMP
static inline int strncmp(const char *str1, const char *str2, size_t n)
{
    int d0, d1, d2, d3;
    asm volatile(
        "0:\t decl %%ecx\n\t"
        "js 3f\n\t"
        "lodsb\n\t"
        "scasb\n\t"
        "jne 1f\n\t"
        "testb %%al, %%al\n\t"
        "jne 0b\n\t"
        "3:\t xorl %%eax, %%eax\n\t"
        "jmp 2f\n\t"
        "1:\t sbbl %%eax, %%eax\n\t"
        "orb $1, %%al\n\t"
        "2:\t"
        : "=a"(d0), "=&S"(d1), "=&D"(d2), "=&c"(d3)
        : "1"(str1), "2"(str2), "3"(n)
        : "memory");
    return d0;
}

#endif
