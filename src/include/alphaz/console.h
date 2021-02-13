#ifndef _ALPHAZ_CONSOLE_H_
#define _ALPHAZ_CONSOLE_H_

#include <alphaz/type.h>

struct console_desc {
    int width, height;
    unsigned int *buf;
    unsigned int pixel_size;        // 每一个像素占用的字节数
};

extern struct console_desc console;

void console_init(void);
void clear_screen(void);
ssize_t console_write(const char *, size_t, unsigned char);

#endif
