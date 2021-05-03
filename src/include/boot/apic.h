#ifndef _BOOT_APIC_H_
#define _BOOT_APIC_H_

// clang-format off

#define APIC_IDR           0x20 /* Local APIC ID Register */
#define APIC_LVR           0x30 /* Local APIC Version Register */
#define APIC_TPR           0x80 /* Task Priority Register */
#define APIC_EOI           0xb0 /* EOI Register */
#define APIC_SVR           0xf0 /* Spurious Interrupt Vector Register */
    #define APIC_SVR_ENABLE    0x100
#define APIC_ESR           0x280 /* Error Status Register */
#define APIC_ICRLO         0x300
#define APIC_ICRHI         0x310
    #define APIC_ICR_BCAST  0x80000
    #define APIC_ICR_INTR   0x500
    #define APIC_ICR_ASSERT 0x4000
    #define APIC_ICR_LEVEL  0x8000
    #define APIC_ICR_DELIVS 0x1000
    #define APIC_ICR_STARTUP 0x600
#define APIC_LVTT          0x320 /* LVT Timer Register */
    #define APIC_LVTT_PERIODIC (1 << 17)
#define APIC_LVTPC         0x340 /* LVT Performance Monitoring Counters Register */
#define APIC_LVTLINE0      0x350 /* LVT LINT0 Register */
#define APIC_LVTLINE1      0x360 /* LVT LINT1 Register */
#define APCI_LVTER         0x370 /* LVT Error Register */
#define APIC_TICR          0x380 /* Initial Counter Register (for timer) */
#define APIC_TCCR          0x390 /* Current Count Register (for timer) */
#define APIC_TDCR          0x3e0 /* Divide Configuration Register (for timer) */
    #define APIC_TDCR_D1       0xb   /* divide counts by 1 */

#define APIC_LVT_MASK         (1 << 16)
#define APIC_TIMER_INIT_COUNT 10000000

/* IRQ */
#define APIC_TIMER_IRQ    0x20 /* Local Timer irq */
#define APIC_IRQ_ERROR    0x30
#define APIC_IRQ_SPURIOUS 0x3f
#define IOAPIC_IRQ_BASE   0x20

#define IOREGSEL 0x00
#define IOWIN    0x10
#define IOEOI    0x40

#define IOAPIC_ID     0x00
#define IOAPIC_VER    0x01
#define IOAPIC_REDTBL 0x10
    #define IOAPIC_RED_MASK 0x10000

#ifndef __ASSEMBLY__

void apic_init(void);
int lapic_init(void);

unsigned int apic_read(unsigned long off);
void         apic_write(unsigned long off, unsigned int value);
unsigned int ioapic_read(unsigned long reg);
void ioapic_write(unsigned long reg, unsigned int value);

extern unsigned int ioapic_maxintr;

#endif

#endif
