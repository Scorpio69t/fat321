#include <boot/apic.h>
#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/irq.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>

#define NR_HW_IRQ 0x10
static pid_t hw_proc[NR_HW_IRQ];

static void setup_idt_desc(int nr, uint64 addr, uint8 dpl)
{
    struct gate_struct *gate = &((struct gate_struct *)idt_table)[nr];
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
    /* trap and exception */
    setup_idt_desc(0x00, (uint64)divide_error, 0);
    setup_idt_desc(0x01, (uint64)single_step_exception, 0);
    setup_idt_desc(0x02, (uint64)nmi, 0);
    setup_idt_desc(0x03, (uint64)breakpoint_exception, 3);
    setup_idt_desc(0x04, (uint64)overflow, 3);
    setup_idt_desc(0x05, (uint64)bounds_check, 0);
    setup_idt_desc(0x06, (uint64)inval_opcode, 0);
    setup_idt_desc(0x07, (uint64)copr_not_available, 0);
    setup_idt_desc(0x08, (uint64)double_fault, 0);
    setup_idt_desc(0x09, (uint64)copr_seg_overrun, 0);
    setup_idt_desc(0x0a, (uint64)inval_tss, 0);
    setup_idt_desc(0x0b, (uint64)segment_not_present, 0);
    setup_idt_desc(0x0c, (uint64)stack_exception, 0);
    setup_idt_desc(0x0d, (uint64)general_protection, 0);
    setup_idt_desc(0x0e, (uint64)page_fault, 0);
    setup_idt_desc(0x10, (uint64)copr_error, 0);

    /* hardware intr */
    setup_idt_desc(0x20, (uint64)apic_timer, 0);
    setup_idt_desc(0x21, (uint64)hwint0x21, 0);
    setup_idt_desc(0x22, (uint64)hwint0x22, 0);
    setup_idt_desc(0x23, (uint64)hwint0x23, 0);
    setup_idt_desc(0x24, (uint64)hwint0x24, 0);
    setup_idt_desc(0x25, (uint64)hwint0x25, 0);
    setup_idt_desc(0x26, (uint64)hwint0x26, 0);
    setup_idt_desc(0x27, (uint64)hwint0x27, 0);
    setup_idt_desc(0x28, (uint64)hwint0x28, 0);
    setup_idt_desc(0x29, (uint64)hwint0x29, 0);
    setup_idt_desc(0x2a, (uint64)hwint0x2a, 0);
    setup_idt_desc(0x2b, (uint64)hwint0x2b, 0);
    setup_idt_desc(0x2c, (uint64)hwint0x2c, 0);
    setup_idt_desc(0x2d, (uint64)hwint0x2d, 0);
    setup_idt_desc(0x2e, (uint64)hwint0x2e, 0);
    setup_idt_desc(0x2f, (uint64)hwint0x2f, 0);

    setup_idt_desc(APIC_IRQ_SPURIOUS, (uint64)spurious_intr, 0);
    setup_idt_desc(APIC_IRQ_ERROR, (uint64)apic_error, 0);
    /* sysytem call */
    setup_idt_desc(0x80, (uint64)system_call, 3);
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
    setup_irq();
    setup_idtr();
    memset(hw_proc, 0x00, sizeof(hw_proc));
}

int register_irq(unsigned vector, pid_t pid)
{
    if (vector >= IOAPIC_IRQ_BASE + ioapic_maxintr || vector <= IOAPIC_IRQ_BASE)
        return -1;
    if (hw_proc[vector - IOAPIC_IRQ_BASE] != 0)
        return -1;
    hw_proc[vector - IOAPIC_IRQ_BASE] = pid;
    ioapic_write(IOAPIC_REDTBL + (vector - IOAPIC_IRQ_BASE) * 2 + 1, 0);  // TODO: 考虑投递到哪个CPU
    ioapic_write(IOAPIC_REDTBL + (vector - IOAPIC_IRQ_BASE) * 2,
                 vector); /* 使用边沿触发，也就不用IOAPIC发送EOI消息了 */
    return 0;
}

int unregister_irq(unsigned vector)
{
    if (vector >= IOAPIC_IRQ_BASE + ioapic_maxintr || vector <= IOAPIC_IRQ_BASE)
        return -1;
    hw_proc[vector - IOAPIC_IRQ_BASE] = 0;
    ioapic_write(IOAPIC_REDTBL + (vector - IOAPIC_IRQ_BASE) * 2 + 1, 0);
    ioapic_write(IOAPIC_REDTBL + (vector - IOAPIC_IRQ_BASE) * 2, IOAPIC_RED_MASK);
    return 0;
}

void do_timer(frame_t *reg)
{
    ticks_plus();
    update_alarm();
    current->flags |= NEED_SCHEDULE;
}

static void do_hwint(frame_t *regs, unsigned nr)
{
    int     ind = nr - 0x20;
    message msg = {.type = MSG_INTR,
                   .src = IPC_INTR,
                   .m_intr = {
                       .type = INTR_OK,
                   }};

    if (hw_proc[ind] == 0) {
        printk("no register hw intr\n");
        return;
    }
    do_send(regs, hw_proc[ind], &msg);
}

/* 异常的统一处理函数 */
void exception_handler(frame_t *regs, unsigned nr)
{
    unsigned long addr;
    asm volatile("movq %%cr2, %0" : "=r"(addr)::"memory");
    printk("Exception ---> %d addr %llx\n", nr, regs->rip);
}

void do_IRQ(frame_t *regs)
{
    uint64 vector = regs->orig_rax;
    switch (vector) {
    case 0x00 ... 0x10:
        exception_handler(regs, vector);
        break;
    case 0x21 ... 0x2f:
        do_hwint(regs, vector);
        break;
    default:
        break;
    }
}
