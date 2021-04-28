#include <boot/apic.h>
#include <boot/cpu.h>
#include <boot/io.h>
#include <kernel/bugs.h>
#include <kernel/kernel.h>

unsigned long apic_base;
unsigned char ioapicid; /* init in mptable.c */

unsigned int apic_read(unsigned long off)
{
    return *((volatile unsigned int *)(apic_base + off));
}

void apic_write(unsigned long off, unsigned int value)
{
    *((volatile unsigned int *)(apic_base + off)) = value;
}

int get_apic_version(void)
{
    int v;

    v = apic_read(APIC_LVR);
    return v & 0xff;
}

int get_maxlvt(void)
{
    int val;

    val = apic_read(APIC_LVR);
    return (val >> 16) & 0xff;
}

void apic_init(void)
{
    int           eax, ebx, ecx, edx;
    unsigned long val;
    int           maxlvt;

    /* check whether APIC is supported */
    cpuid(0x01, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 9))) {
        panic("CPU not support APIC\n");
        goto failed;
    }
    /* enable APIC mode in IMCR */
    outb(0x22, 0x70);
    outb(0x23, inb(0x23) | 0x1);
    /* enable APIC */
    rdmsr(MSR_IA32_APIC_BASE, val);
    val = val | (1UL << 11);
    wrmsr(MSR_IA32_APIC_BASE, val);
    apic_base = val & -((unsigned long)(1 << 12));
    apic_base = to_vir(apic_base);
    printk("APIC base %llx\n", apic_base);
    if (get_apic_version() < 0x10) {
        panic("CPU support integrated APIC\n");
        goto failed;
    }

    maxlvt = get_maxlvt();
    /* enable SVR */
    apic_write(APIC_SVR, 0x100 | 0x3f);
    /* setup timer */
    apic_write(APIC_TDCR, APIC_TDCR_D1);
    apic_write(APIC_LVTT, APIC_LVTT_PERIODIC | APIC_TIMER_IRQ);
    apic_write(APIC_TICR, APIC_TIMER_INIT_COUNT);

    /* disable logical interrupt lines */
    apic_write(APIC_LVTLINE0, APIC_LVT_MASK);
    apic_write(APIC_LVTLINE1, APIC_LVT_MASK);

    /* disable Performance Monitoring Counters Register */
    if (maxlvt >= 4)
        apic_write(APIC_LVTPC, APIC_LVT_MASK);

    /* setup Error Register */
    apic_write(APCI_LVTER, APIC_IRQ_ERROR);

    /* clear ESR */
    apic_write(APIC_ESR, 0);
    apic_write(APIC_ESR, 0);

    /* clear EOI Register */
    apic_write(APIC_EOI, 0);

    /* setup TPR */
    apic_write(APIC_TPR, 0);

    printk("apic init finish\n");
    return;
failed:
    cpu_idle();
}
