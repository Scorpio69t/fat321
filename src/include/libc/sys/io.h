#ifndef _SYS_IO_H_
#define _SYS_IO_H_

#include <sys/types.h>

static inline void outb(u16 port, u8 value)
{
    asm volatile(
        "outb %%al, %%dx\n\t"
        "nop\n\t"
        "nop\n\t"
        : "=&a"(value), "=&d"(port)
        : "0"(value), "1"(port));
}

static inline u8 inb(u16 port)
{
    u8 val;
    asm volatile(
        "inb %%dx, %%al\n\t"
        "nop\n\t"
        "nop\n\t"
        : "=&a"(val), "=&d"(port)
        : "1"(port));
    return val;
}

/**
 * innb - 从指定端口读取n个字节
 */
static inline void innb(unsigned short port, const void *buf, size_t n)
{
    asm volatile(
        "cld\n\t"
        "rep; insb\n\t"
        "mfence\n\t"
        :
        : "d"(port), "D"(buf), "c"(n)
        : "memory");
}

/**
 * outnb - 向指定端写n个字节
 */
static inline void outnb(unsigned short port, const void *buf, size_t n)
{
    asm volatile(
        "cld\n\t"
        "rep; outsb\n\t"
        "mfence\n\t"
        :
        : "d"(port), "S"(buf), "c"(n)
        : "memory");
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
        : "d"(port), "D"(buf), "c"(n)
        : "memory");
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
        : "d"(port), "S"(buf), "c"(n)
        : "memory");
}

static inline uint32 readl(uint64 addr)
{
    uint32 d0;
    asm volatile("movl (%%rsi), %%eax\n\t" : "=&a"(d0) : "S"(addr));
    return d0;
}

static inline uint8 readb(uint64 addr)
{
    uint8 d0;
    asm volatile("movb (%%rsi), %%al\n\t" : "=&a"(d0) : "S"(addr));
    return d0;
}

static inline uint64 readq(uint64 addr)
{
    uint64 d0;
    asm volatile("movb (%%rsi), %%rax\n\t" : "=&a"(d0) : "S"(addr));
    return d0;
}

#endif
