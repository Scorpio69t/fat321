#include <boot/console.h>
#include <kernel/console.h>
#include <kernel/kernel.h>
#include <kernel/stdio.h>
#include <kernel/unistd.h>
#include <stdarg.h>

static unsigned char printk_color[] = {
    0x0f, 0x0f, 0x0f, 0x0c, 0x0f, 0x0f, 0x0f, 0x0f,
};

/**
 * printk - 内核态字符串格式化输出函数
 */
int printk(const char *fmt, ...)
{
    static char buf[1024];
    va_list     args;
    char *      p = buf;
    int         level, len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (p[0] == '<' && p[1] >= '0' && p[1] <= '7' && p[2] == '>') {
        level = p[1] - '0';
        p += 3;
        len -= 3;
    } else
        level = 6; /* infomation level */

    console_write(p, len, printk_color[level]);

    return len;
}
