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
#define APIC_TIMER_IRQ        0x20 /* Local Timer irq */
#define APIC_IRQ_ERROR        0x30
#define APIC_IRQ_SPURIOUS     0x3f
#define APIC_TIMER_INIT_COUNT 10000000

#ifndef __ASSEMBLY__

void         apic_init(void);
unsigned int apic_read(unsigned long off);
void         apic_write(unsigned long off, unsigned int value);

#endif

#endif
