#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <types.h>

#define EOF 0

void stdio_init(void);

int vsprintf(char *buf, const char *fmt, va_list args);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...);

#endif
