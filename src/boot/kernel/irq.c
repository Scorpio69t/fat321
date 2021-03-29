#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/i8259.h>
#include <boot/irq.h>
#include <kernel/kernel.h>
#include <kernel/linkage.h>

static void setup_idt_desc(struct gate_struct *gate, uint64 addr, uint8 dpl)
{
    gate->offset0 = addr & 0xffff;
    gate->segment = KERNEL_CODE_DESC;
    gate->type = 0xe;
    gate->dpl = dpl & 0x3;
    gate->p = 1;
    gate->offset1 = (addr >> 16) & 0xffff;
    gate->offset2 = addr >> 32;
}

static void setup_irq(void)
{
    int i;
    for (i = 0; i < NR_IRQ; i++) {
        struct irq_struct *irq = &irq_array[i];
        if (irq_array[i].state & IRQ_STATE_DEFINED) {
            setup_idt_desc((void *)idt_table + sizeof(struct gate_struct) * irq->vector, (uint64)irq->entry, irq->ring);
        }
    }
}

static inline void setup_idtr(void)
{
    static struct idtr_struct idtr;
    idtr.len = idt_end - idt_table - 1;
    idtr.base = (uint64)idt_table;
    asm volatile("lidt %0" ::"m"(idtr));
}

void irq_init(void)
{
    init_8259A();
    setup_irq();
    setup_idtr();
}

int register_irq(unsigned vector, irq_handler handler)
{
    if (vector >= NR_IRQ)
        return -1;
    if (!(irq_array[vector].state & IRQ_STATE_DEFINED) || irq_array[vector].state & IRQ_STATE_INUSE)
        return -1;
    irq_array[vector].state |= IRQ_STATE_INUSE;
    irq_array[vector].handler = handler;
    if (vector >= 0x20 && vector <= 0x2f) {
        enable_irq(vector - 0x20);
    }
    return 0;
}

int unregister_irq(unsigned vector)
{
    if (vector >= NR_IRQ)
        return -1;
    if (vector >= 0x20 && vector <= 0x2f) {
        disable_irq(vector - 0x20);
    }
    irq_array[vector].state = 0x01;
    irq_array[vector].handler = NULL;
    return 0;
}

void do_IRQ(struct pt_regs *regs)
{
    uint64 vector = regs->orig_rax;
    if (irq_array[vector].state & IRQ_STATE_INUSE)
        irq_array[vector].handler(regs, vector);
}

/* 异常的统一处理函数 */
static void exception_handler(struct pt_regs *regs, unsigned nr)
{
    printk("Exception ---> %d\n", nr);
}

/* 硬件中断的统一处理函数 */
static void spurious_irq(struct pt_regs *regs, unsigned nr)
{
    printk("spurious_irq: %d\n", nr);
}

struct irq_struct irq_array[NR_IRQ] = {
    /* 处理器异常 */
    [0x00] = {.state = 0x03, .entry = divide_error, .handler = exception_handler, .vector = 0x00, .ring = RING0},
    [0x01] =
        {.state = 0x03, .entry = single_step_exception, .handler = exception_handler, .vector = 0x01, .ring = RING0},
    [0x02] = {.state = 0x03, .entry = nmi, .handler = exception_handler, .vector = 0x02, .ring = RING0},
    [0x03] =
        {.state = 0x03, .entry = breakpoint_exception, .handler = exception_handler, .vector = 0x03, .ring = RING3},
    [0x04] = {.state = 0x03, .entry = overflow, .handler = exception_handler, .vector = 0x04, .ring = RING3},
    [0x05] = {.state = 0x03, .entry = bounds_check, .handler = exception_handler, .vector = 0x05, .ring = RING0},
    [0x06] = {.state = 0x03, .entry = inval_opcode, .handler = exception_handler, .vector = 0x06, .ring = RING0},
    [0x07] = {.state = 0x03, .entry = copr_not_available, .handler = exception_handler, .vector = 0x07, .ring = RING0},
    [0x08] = {.state = 0x03, .entry = double_fault, .handler = exception_handler, .vector = 0x08, .ring = RING0},
    [0x09] = {.state = 0x03, .entry = copr_seg_overrun, .handler = exception_handler, .vector = 0x09, .ring = RING0},
    [0x0a] = {.state = 0x03, .entry = inval_tss, .handler = exception_handler, .vector = 0x0a, .ring = RING0},
    [0x0b] = {.state = 0x03, .entry = segment_not_present, .handler = exception_handler, .vector = 0x0b, .ring = RING0},
    [0x0c] = {.state = 0x03, .entry = stack_exception, .handler = exception_handler, .vector = 0x0c, .ring = RING0},
    [0x0d] = {.state = 0x03, .entry = general_protection, .handler = exception_handler, .vector = 0x0d, .ring = RING0},
    [0x0e] = {.state = 0x03, .entry = page_fault, .handler = exception_handler, .vector = 0x0e, .ring = RING0},

    [0x10] = {.state = 0x03, .entry = copr_error, .handler = exception_handler, .vector = 0x10, .ring = RING0},

    /* 外部硬件中断 */
    [0x20] = {.state = 0x01, .entry = hwint0x20, .handler = spurious_irq, .vector = 0x20, .ring = RING0},
    [0x21] = {.state = 0x01, .entry = hwint0x21, .handler = spurious_irq, .vector = 0x21, .ring = RING0},
    [0x22] = {.state = 0x01, .entry = hwint0x22, .handler = spurious_irq, .vector = 0x22, .ring = RING0},
    [0x23] = {.state = 0x01, .entry = hwint0x23, .handler = spurious_irq, .vector = 0x23, .ring = RING0},
    [0x24] = {.state = 0x01, .entry = hwint0x24, .handler = spurious_irq, .vector = 0x24, .ring = RING0},
    [0x25] = {.state = 0x01, .entry = hwint0x25, .handler = spurious_irq, .vector = 0x25, .ring = RING0},
    [0x26] = {.state = 0x01, .entry = hwint0x26, .handler = spurious_irq, .vector = 0x26, .ring = RING0},
    [0x27] = {.state = 0x01, .entry = hwint0x27, .handler = spurious_irq, .vector = 0x27, .ring = RING0},
    [0x28] = {.state = 0x01, .entry = hwint0x28, .handler = spurious_irq, .vector = 0x28, .ring = RING0},
    [0x29] = {.state = 0x01, .entry = hwint0x29, .handler = spurious_irq, .vector = 0x29, .ring = RING0},
    [0x2a] = {.state = 0x01, .entry = hwint0x2a, .handler = spurious_irq, .vector = 0x2a, .ring = RING0},
    [0x2b] = {.state = 0x01, .entry = hwint0x2b, .handler = spurious_irq, .vector = 0x2b, .ring = RING0},
    [0x2c] = {.state = 0x01, .entry = hwint0x2c, .handler = spurious_irq, .vector = 0x2c, .ring = RING0},
    [0x2d] = {.state = 0x01, .entry = hwint0x2d, .handler = spurious_irq, .vector = 0x2d, .ring = RING0},
    [0x2e] = {.state = 0x01, .entry = hwint0x2e, .handler = spurious_irq, .vector = 0x2e, .ring = RING0},
    [0x2f] = {.state = 0x01, .entry = hwint0x2f, .handler = spurious_irq, .vector = 0x2f, .ring = RING0},

    /* 系统调用中断, 注意，这里系统调用不经过do_IRQ, 而是根据系统调用表调用相关的handler */
    [0x80] = {.state = 0x01, .entry = system_call, .vector = 0x80, .ring = RING3},

};
