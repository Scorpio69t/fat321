/* 初始化8259 */
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/i8259.h>

void init_8259A()
{
    /* 主片 icw1 */
    outb(INT_M_CTL, 0x11);
    /* 从片 icw1 */
    outb(INT_S_CTL, 0x11);
    /* 主片 icw2 设置中断入口地址为0x20 */
    outb(INT_M_CTLMASK, INT_VECTOR_IRQ0);
    /* 从片 icw2 设置中断入口地址为0x28 */
    outb(INT_S_CTLMASK, INT_VECTOR_IRQ8);
    /* 主片 icw3 ir2对应从片 */
    outb(INT_M_CTLMASK, 0x4);
    /* 从片 icw3 对应主片ir2 */
    outb(INT_S_CTLMASK, 0x2);
    /* 主片 icw4 */
    outb(INT_M_CTLMASK, 0x1);
    /* 从片 icw4 */
    outb(INT_S_CTLMASK, 0x1);
    /* 主片 ocw1 */
    outb(INT_M_CTLMASK, 0xff);
    /* 从片 ocw1 */
    outb(INT_S_CTLMASK, 0xff);
}


/**
 * enable - 开启8259a的某个引脚
 * @irq: 要开启的8259a的引脚的编号
 */
void enable_irq(unsigned short irq)
{
    u8 val;

    if(irq < 0 || irq > 15)
        return;
    val = ~(1 << (irq % 8));
    if (irq <= 7) {
        val = val & inb(INT_M_CTLMASK);
        outb(INT_M_CTLMASK, val);
    } else {
        val = val & inb(INT_S_CTLMASK);
        outb(INT_S_CTLMASK, val);
        val = (~(1 << 2)) & inb(INT_M_CTLMASK);     /* 开启主片级联 */
        outb(INT_M_CTLMASK, val);
    }
}


/**
 * disable - 关闭8259a的某个引脚
 * @irq: 要关闭的8259a的引脚的编号
 */
void disable_irq(unsigned short irq)
{
    u8 val;

    if(irq < 0 || irq > 15)
        return;
    val = 1 << (irq % 8);
    if (irq <= 7) {
        val = val | inb(INT_M_CTLMASK);
        outb(INT_M_CTLMASK, val);
    } else {
        val = val | inb(INT_S_CTLMASK);
        outb(INT_S_CTLMASK, val);
    }
}
