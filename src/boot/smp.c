#include <boot/apic.h>
#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/smp.h>
#include <kernel/bugs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/string.h>

int          current_boot_cpu;
volatile int smp_booting;

extern unsigned long init_stack;

extern unsigned char smp_boot_start[];
extern unsigned char smp_boot_end[];
unsigned char *      smp_boot_base;

proc_t *make_idle()
{
    proc_t *idle;

    idle = (proc_t *)__get_free_pages(GFP_KERNEL, KERNEL_STACK_ORDER);
    assert(idle != NULL);
    idle->mm.pgd = kinfo.global_pgd_start;
    return idle;
}

int do_smp_boot(int apicid)
{
    printk("booting cpu%d...\n", apicid);
    if (!proc_idle[apicid])
        proc_idle[apicid] = make_idle();

    smp_booting = 1;
    init_stack = (unsigned long)proc_idle[apicid] + KERNEL_STACK_SIZE - 8;

    apic_write(APIC_ICRHI, apicid << 24);
    apic_write(APIC_ICRLO, APIC_ICR_INTR | APIC_ICR_LEVEL | APIC_ICR_ASSERT);
    while (apic_read(APIC_ICRLO) & APIC_ICR_DELIVS) nop();

    apic_write(APIC_ICRHI, apicid << 24);
    apic_write(APIC_ICRLO, APIC_ICR_STARTUP | (SMP_BOOT_BASE >> 12));
    while (apic_read(APIC_ICRLO) & APIC_ICR_DELIVS) nop();
    while (smp_booting) nop();

    printk("xxxxx\n");

    return 0;
}

void smp_init(void)
{
    int apicid;

    if (nr_cpu <= 1)
        return;

    /* copy boot code to the correct positon */
    smp_boot_base = (unsigned char *)to_vir(SMP_BOOT_BASE);
    memcpy(smp_boot_base, smp_boot_start, smp_boot_end - smp_boot_start);

    for (apicid = 0; apicid < nr_cpu; apicid++) {
        if (apicid == boot_apic_id)
            continue;
        current_boot_cpu = apicid;
        do_smp_boot(apicid);
    }
}

void smp_boot_main()
{
    printk("==== smp boot %d\n", current_boot_cpu);
    smp_booting = 0;
    barrier();
    cpu_idle();
}
