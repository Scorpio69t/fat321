#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <sys/types.h>

#define EOF 0

int vsprintf(char *buf, const char *fmt, va_list args);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...);

int printf(const char *fmt, ...);

#endif
