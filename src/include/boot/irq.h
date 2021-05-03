#ifndef _BOOT_INT_H_
#define _BOOT_INT_H_

#ifndef __ASSEMBLY__
#include <boot/cpu.h>

/* idt_table and idt_end are defined in head.S */
extern uint8 idt_table[];
extern uint8 idt_end[];

int register_irq(unsigned, pid_t);
int unregister_irq(unsigned vector);

struct idtr_struct {
    uint16 len;
    uint64 base;
} __attribute__((packed));

/* 中断向量数 */
#define NR_IRQ 256
extern struct gate_struct idt[];

void irq_init();
void smp_irq_init(void);
/*
 * 开中断 IF=1
 */
static inline void enable_interrupt(void)
{
    asm volatile("sti");
}

/*
 * 关中断 IF=0
 */
static inline void disable_interrupt(void)
{
    asm volatile("cli");
}

/**
 * 获取中断允许标志位
 */
static inline int get_if(void)
{
    int d0;
    asm volatile(
        "pushf\n\t"
        "pop %%eax\n\t"
        : "=&a"(d0));
    return (d0 & 512);  // 1<<9 if在标志寄存器右起第十位
}

extern void divide_error();
extern void single_step_exception();
extern void nmi();
extern void breakpoint_exception();
extern void overflow();
extern void bounds_check();
extern void inval_opcode();
extern void copr_not_available();
extern void double_fault();
extern void copr_seg_overrun();
extern void inval_tss();
extern void segment_not_present();
extern void stack_exception();
extern void general_protection();
extern void page_fault();
extern void copr_error();

extern void apic_timer();
extern void hwint0x21();
extern void hwint0x22();
extern void hwint0x23();
extern void hwint0x24();
extern void hwint0x25();
extern void hwint0x26();
extern void hwint0x27();
extern void hwint0x28();
extern void hwint0x29();
extern void hwint0x2a();
extern void hwint0x2b();
extern void hwint0x2c();
extern void hwint0x2d();
extern void hwint0x2e();
extern void hwint0x2f();
extern void spurious_intr();
extern void apic_error();
extern void system_call(void);

#endif /*__ASSEMBLY__*/

#endif
