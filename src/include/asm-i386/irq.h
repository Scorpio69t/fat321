#ifndef _ASM_INT_H_
#define _ASM_INT_H_

#ifndef __ASSEMBLY__
#include <asm/cpu.h>
#include <asm/i8259.h>

typedef void (*irq_entry) ();
typedef void (*irq_handler) (struct pt_regs *, unsigned);

struct irq_struct
{
    volatile long state;     /* 中断状态 */
    irq_entry entry;         /* 中断入口 */
    irq_handler handler;     /* 中断处理函数 */
    unsigned long vector;    /* 中断向量 */
    unsigned short ring;     /* 中断特权级 */
};

int register_irq(unsigned, irq_handler);
int unregister_irq(unsigned vector);

#define IRQ_STATE_DEFINED   (1 << 0)
#define IRQ_STATE_INUSE     (1 << 1)

struct idtr_struct
{
    u16 len;
    u32 base;
} __attribute__((packed));

/* 中断向量数 */
#define NR_IRQ    256
extern struct gate_struct idt[];
extern struct irq_struct irq_array[];

void irq_init();
extern void enable_irq(unsigned short);
extern void disable_irq(unsigned short);

/*
 * 开中断 IF=1
 */
static inline void sti(void)
{
    asm volatile("sti");
}


/*
 * 关中断 IF=0
 */
static inline void cli(void)
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
        :"=&a"(d0));
    return (d0 & 512);      // 1<<9 if在标志寄存器右起第十位
}


/* cpu异常处理函数，定义在arch/../kernel/entry.asm */
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

/* 硬件中断的处理函数，对应8259a的15个引脚 */
extern void hwint0x20();
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


/* 系统调用号 */
#define INT_VECTOR_SYSCALL          0x80
/* defined in entry.asm */
extern void system_call(void);

#endif  /*__ASSEMBLY__*/

#endif
