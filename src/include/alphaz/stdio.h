#ifndef _ALPHAZ_STDIO_H_
#define _ALPHAZ_STDIO_H_

/**
 * stdarg.h是GCC的一部分，其中的va_list也是GCC的内建类型，且定义了一组宏用于处理具有可变参
 * 数的函数。该头文件可以直接使用，无需特别处理编译和链接
 */
#include <stdarg.h>
#include <alphaz/type.h>
#include <alphaz/fs.h>

#define EOF         0

extern struct file *stdin;
extern struct file *stdout;
extern struct file *stderr;

void stdio_init(void);

int vsprintf(char *buf, const char *fmt, va_list args);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...);

int printf(const char *fmt, ...);

#endif
