#include <boot/bug.h>
#include <boot/console.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/string.h>
#include <kernel/config.h>
#include <kernel/console.h>
#include <kernel/mm.h>


#define ROW 25
#define COL 80

/**
 * 获取屏幕当前的游标
 * CRT Controller Registers     Address Register    0x3d4
 *                              Data    Register    0x3d5
 *
 *      Cursor Start Register   0x0a
 *      Cursor End   Register   0x0b
 */
unsigned short get_cursor(void)
{
    unsigned short cur;

    outb(0x3d4, 0x0e);
    cur = inb(0x3d5);
    outb(0x3d4, 0x0f);
    cur = (cur << 8) | inb(0x3d5);
    return cur;
}

/**
 * 设置屏幕当前光标
 * @cur: 要设置的当前光标位置
 */
void set_cursor(unsigned short cur)
{
    outb(0x3d4, 0x0e);
    outb(0x3d5, (cur >> 8) & 0xff);
    outb(0x3d4, 0x0f);
    outb(0x3d5, cur & 0xff);
}

/**
 * 屏幕向上卷动指定的行数
 * @line: 卷动的行数
 */
int console_curl(int line)
{
    unsigned long b0, b1;

    if (line < 0 || line > 25)
        return -1;

    b0 = DEFAULT_VIDEO_BASE;
    b1 = DEFAULT_VIDEO_BASE + 160 * line;
    memcpy((void *)b0, (void *)b1, 160 * (25 - line));
    b0 = DEFAULT_VIDEO_BASE + 160 * 24;
    memset((void *)b0, 0, 160);
    return 0;
}

/**
 * 向屏幕上指定位置处写一个字符
 * @c: 字符
 * @type: 字符的颜色属性
 * @cur: 要写的位置
 */
void write_char(char c, unsigned char type, unsigned short cur)
{
    unsigned short val = (unsigned short)c | ((unsigned short)type << 8);
    unsigned long  pos = cur * 2 + DEFAULT_VIDEO_BASE;

    asm volatile("movw %%ax, (%%rdi)" ::"a"(val), "D"(pos));
}


/**
 * 向控制台写字符串
 * @buf: 字符串缓冲区
 * @n: 字符串长度
 * @type: 字符串颜色属性
 */
ssize_t console_write(const char *buf, size_t n, unsigned char type)
{
    int i, cur;

    cur = get_cursor();
    for (i = 0; i < n; i++) {
        switch (buf[i]) {
        case '\n':
            cur = (cur / COL + 1) * COL;
            break;
        case '\t':
            cur = cur + 4;
            break;
        case '\b':
            cur = cur - 1;
            write_char(' ', type, (unsigned short)cur);
            break;
        default:
            write_char(buf[i], type, (unsigned short)cur);
            cur++;
        }
        if (cur >= ROW * COL) {
            cur = (ROW - 1) * COL;
            console_curl(1);
        }
        set_cursor(cur);
    }
    return n;
}

