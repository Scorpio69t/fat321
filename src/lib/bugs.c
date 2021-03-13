#include <boot/bug.h>
#include <feng/bugs.h>
#include <feng/kernel.h>
#include <feng/stdio.h>
#include <stdarg.h>

void __panic(char *fmt, char *file, int line, ...)
{
    char buf[128];
    va_list args;
    va_start(args, line);
    vsprintf(buf, fmt, args);
    va_end(args);
    printk(KERN_ERR "panic(%s,%d): %s", file, line, buf);
}

void __assert(char *exp, char *file, int line)
{
    printk(KERN_ERR "assert(%s) -> %s:%d\n", exp, file, line);
}
