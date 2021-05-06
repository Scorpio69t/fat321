#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

int printf(const char *fmt, ...)
{
    static char buf[1024];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return write(1, buf, len);
}

void perror(const char *s)
{
    write(2, (void *)s, strlen(s));
}

int debug(const char *fmt, ...)
{
    static char buf[1024];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    _debug(buf);
    return len;
}
