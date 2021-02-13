#include <stdarg.h>
#include <alphaz/unistd.h>
#include <alphaz/stdio.h>
#include <alphaz/kernel.h>
#include <alphaz/console.h>

#include <asm/console.h>


/**
 * printf - 用户态字符串格式化输出函数
 *
 * 该函数使用write系统调用进行输出，只能用于用户态，不能用于内核态
 */
int printf(const char *fmt, ...)
{
    static char buf[1024];
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, sizeof(buf), fmt, args);
    write(STDOUT_FILENO, buf, i);
    va_end(args);

    return i;
}


static unsigned char printk_color[] = {
    0x0f, 0x0f, 0x0f, 0x0c, 0x0f, 0x0f, 0x0f, 0x0f,
};

/**
 * printk - 内核态字符串格式化输出函数
 *
 * 该函数直接使用tty_write实现，只能用于内核态，不能用于用户态
 */
int printk(const char *fmt, ...)
{
    static char buf[1024];
    va_list args;
    char *p = buf;
    int level, len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (p[0] == '<' && p[1] >= '0' && p[1] <= '7' && p[2] == '>') {
        level = p[1] - '0';
        p += 3;
        len -= 3;
    }
    else
        level = 6;  /* infomation level */

    console_write(p, len, printk_color[level]);

    return len;
}
