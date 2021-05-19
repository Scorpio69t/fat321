
#ifndef _STRING_H_
#define _STRING_H_

#include <sys/types.h>

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

static inline void *memset(void *from, u8 value, size_t n)
{
    uint64 d0, d1;
    u8 d2;
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

static inline void *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && *src != 0; i++, src++) dest[i] = *src;
    return dest;
}

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

static inline int strncmp(const char *str1, const char *str2, size_t n)
{
    size_t i;
    int retval;

    for (i = 0; i < n; i++, str1++, str2++) {
        retval = *str1 - *str2;
        if (retval)
            break;
    }
    return retval;
}

static inline char *strstr(const char *str1, const char *str2)
{
    int len2;

    if (!(len2 = strlen(str2)))
        return (char *)str1;
    for (; *str1 != 0; str1++) {
        if (*str1 == *str2 && strncmp(str1, str2, len2) == 0)
            return (char *)str1;
    }
    return NULL;
}

static inline char *strcat(char *dest, const char *str)
{
    while (*dest) dest++;
    while (*str) *dest++ = *str++;
    *dest = 0;
    return dest;
}

#endif
