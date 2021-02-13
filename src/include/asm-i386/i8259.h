#ifndef _ASM_I8259_H_
#define _ASM_I8259_H_

/* 8259A的初始化端口，主片和从片 */
#define INT_M_CTL     0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL     0xA0
#define INT_S_CTLMASK 0xA1

/* 中断向量 */
#define INT_VECTOR_IRQ0	0x20
#define INT_VECTOR_IRQ8	0x28

void init_8259A();

void enable_irq(unsigned short);
void disable_irq(unsigned short);

#endif
