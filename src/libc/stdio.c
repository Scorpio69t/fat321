#include <stdarg.h>
#include <stdio.h>
#include <sys/syscall.h>

int printf(const char *fmt, ...)
{
    static char buf[1024];
    va_list     args;
    int         len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sys_debug(buf);
    return len;
}
