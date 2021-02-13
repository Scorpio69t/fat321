#ifndef _ASM_IO_H_
#define _ASM_IO_H_

#include <alphaz/type.h>

static inline void outb(u16 port, u8 value)
{
    asm volatile(
        "outb %%al, %%dx\n\t"
        "nop\n\t"
        "nop\n\t"
        : "=&a"(value), "=&d"(port)
        :"0"(value), "1"(port));
}

static inline u8 inb(u16 port)
{
    u8 val;
    asm volatile(
        "inb %%dx, %%al\n\t"
        "nop\n\t"
        "nop\n\t"
        :"=&a"(val), "=&d"(port)
        :"1"(port));
    return val;
}

/**
 * innw - 从指定端口读取n个字
 */
static inline void innw(unsigned short port, const void *buf, size_t n)
{
    asm volatile(
        "cld\n\t"
        "rep; insw\n\t"
        "mfence\n\t"
        :
        :"d"(port), "D"(buf), "c"(n)
        :"memory");
}

/**
 * outnw - 向指定端写n个字
 */
static inline void outnw(unsigned short port, const void *buf, size_t n)
{
    asm volatile(
        "cld\n\t"
        "rep; outsw\n\t"
        "mfence\n\t"
        :
        :"d"(port), "S"(buf), "c"(n)
        :"memory");
}

static inline u32 readl(u32 addr)
{
    u32 d0;
    asm volatile(
        "movl (%%esi), %%eax\n\t"
        :"=&a"(d0)
        :"S"(addr)
    );
    return d0;
}

static inline u8 readb(u32 addr)
{
    u8 d0;
    asm volatile(
        "movb (%%esi), %%al\n\t"
        :"=&a"(d0)
        :"S"(addr)
    );
    return d0;
}

#endif
