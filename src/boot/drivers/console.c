#include <alphaz/mm.h>
#include <alphaz/config.h>
#include <alphaz/console.h>
#include <boot/string.h>
#include <boot/console.h>
#include <boot/irq.h>
#include <boot/io.h>
#include <boot/bug.h>

#ifndef __VBE

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
    cur = (cur<<8) | inb(0x3d5);
    return cur;
}


/**
 * 设置屏幕当前光标
 * @cur: 要设置的当前光标位置
 */
void set_cursor(unsigned short cur)
{
    outb(0x3d4,0x0e);
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
    unsigned long pos = cur * 2 + DEFAULT_VIDEO_BASE;

    asm volatile("movw %%ax, (%%edi)"::"a"(val), "D"(pos));
}

#endif
