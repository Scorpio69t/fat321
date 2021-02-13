#include <alphaz/keyboard.h>
#include <asm/bug.h>
#include <asm/io.h>

/**
 * read_scancode - 从键盘控制器缓冲区读取扫描码
 */
inline unsigned char read_scancode(void)
{
    return inb(0x60);
}

inline void __keyboard_init(void)
{
    while (inb(0x64) & 0x02) nop();  /* 8042缓冲区满则循环 */
    outb(0x64, 0x60);
    while(inb(0x64) & 0x02) nop();
    outb(0x60, 0x65);   /* 使用第一套扫描码，只使能键盘 */
}

/**
 * __sys_reboot - 系统重启
 */
void __sys_reboot(void)
{
    outb(0x64, 0xfe);
}
