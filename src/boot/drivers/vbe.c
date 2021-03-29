#include <kernel/config.h>
#include <kernel/console.h>
#include <kernel/fontdata.h>
#include <kernel/string.h>

#ifdef __VBE

static unsigned int __cur = 0;

#define ROW (console.height / font_8x16.height)
#define COL (console.width / font_8x16.width)

unsigned short get_cursor(void)
{
    return __cur;
}

/**
 * 设置屏幕当前光标
 * @cur: 要设置的当前光标位置
 */
void set_cursor(unsigned short cur)
{
    __cur = cur;
}

/**
 * 屏幕向上卷动指定的行数
 * @line: 卷动的行数
 */
int console_curl(int line)
{
    unsigned long b0, b1;
    unsigned long each_line;  // 每一行占用内存数

    if (line < 0 || line > ROW)
        return -1;

    each_line = console.width * font_8x16.height * line * console.pixel_size;
    b0 = (unsigned long)console.buf;
    b1 = (unsigned long)console.buf + each_line * line;
    memcpy((void *)b0, (void *)b1, each_line * (ROW - line));
    b0 = (unsigned long)console.buf + each_line * (ROW - line);
    memset((void *)b0, 0, each_line);
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
    int            i, j;
    unsigned long  x, y;
    unsigned char *font;

    font = font_8x16.font + (int)c * font_8x16.height;
    x = (cur % COL) * font_8x16.width;
    y = (cur / COL) * font_8x16.height;

    for (i = 0; i < font_8x16.height; i++) {
        for (j = 0; j < font_8x16.width; j++) {
            if (font[i] & (1 << (font_8x16.width - 1 - j)))
                *(console.buf + (y + i) * console.width + (x + j)) = 0xffffffff;
            else
                *(console.buf + (y + i) * console.width + (x + j)) = 0x00000000;
        }
    }
}

#endif
