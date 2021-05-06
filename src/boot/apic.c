#include <boot/apic.h>
#include <boot/cpu.h>
#include <boot/io.h>
#include <kernel/bugs.h>
#include <kernel/kernel.h>

unsigned long apic_base;   /* init in mptable.c */
unsigned long ioapic_base; /* init in mptable.c */
unsigned char ioapicid;    /* init in mptable.c */
unsigned int ioapic_maxintr;

unsigned int apic_read(unsigned long off)
{
    return *((volatile unsigned int *)(apic_base + off));
}

void apic_write(unsigned long off, unsigned int value)
{
    *((volatile unsigned int *)(apic_base + off)) = value;
}

unsigned int ioapic_read(unsigned long reg)
{
    *((volatile unsigned int *)(ioapic_base + IOREGSEL)) = reg;
    return *((volatile unsigned int *)(ioapic_base + IOWIN));
}

void ioapic_write(unsigned long reg, unsigned int value)
{
    *((volatile unsigned int *)(ioapic_base + IOREGSEL)) = reg;
    *((volatile unsigned int *)(ioapic_base + IOWIN)) = value;
}

int get_apic_version(void)
{
    return apic_read(APIC_LVR) & 0xff;
}

int get_maxlvt(void)
{
    return (apic_read(APIC_LVR) >> 16) & 0xff;
}

int ioapic_init(void)
{
    int version, id, i;

    printk("ioapic_base %llx\n", ioapic_base);

    version = ioapic_read(IOAPIC_VER);
    ioapic_maxintr = (version >> 16) & 0xff;
    id = ioapic_read(IOAPIC_ID) >> 24;
    if (id != ioapicid) {
        panic("id isn't equal to ioapicid\n");
        return -1;
    }

    /* init all pin */
    for (i = 0; i < ioapic_maxintr; i++) {
        ioapic_write(IOAPIC_REDTBL + 2 * i, IOAPIC_RED_MASK | (IOAPIC_IRQ_BASE + i));
        ioapic_write(IOAPIC_REDTBL + 2 * i + 1, 0);
    }
    printk("ioapic maxintr: %d\n", ioapic_maxintr);
    return 0;
}

int lapic_init(void)
{
    int maxlvt;

    maxlvt = get_maxlvt();
    /* enable SVR */
    apic_write(APIC_SVR, APIC_SVR_ENABLE | APIC_IRQ_SPURIOUS);
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

    return 0;
}

void apic_init(void)
{
    int eax, ebx, ecx, edx;
    unsigned char val;

    /* check whether APIC is supported */
    cpuid(0x01, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 9))) {
        panic("CPU unsupport APIC\n");
        goto failed;
    }
    /* enable APIC mode in IMCR */
    outb(0x22, 0x70);
    val = inb(0x23) | 0x01;
    outb(0x23, val);

    /* local apic can also be enabled using this code */
    // rdmsr(MSR_IA32_APIC_BASE, val);
    // val = val | (1UL << 11);
    // wrmsr(MSR_IA32_APIC_BASE, val);
    // apic_base = val & -((unsigned long)(1 << 12));
    // apic_base = to_vir(apic_base);

    printk("APIC base %llx\n", apic_base);
    if (get_apic_version() < 0x10) {
        panic("CPU unsupport integrated APIC\n");
        goto failed;
    }
    if (lapic_init() != 0)
        goto failed;
    if (ioapic_init() != 0)
        goto failed;
    return;
failed:
    cpu_idle();
}
