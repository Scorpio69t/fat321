#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

void __panic(char *fmt, char *file, int line, ...)
{
    char    buf[128];
    va_list args;
    va_start(args, line);
    vsprintf(buf, fmt, args);
    va_end(args);
    debug(
        "<3>"
        "panic(%s,%d): %s",
        file, line, buf);
}

void __assert(char *exp, char *file, int line)
{
    debug(
        "<3>"
        "assert(%s) -> %s:%d\n",
        exp, file, line);
}
