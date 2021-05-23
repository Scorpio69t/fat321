#include <boot/apic.h>
#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/smp.h>
#include <kernel/bugs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/string.h>

int current_boot_cpu = 0; /* which cpu is booting */

volatile int smp_booting; /* is there a cpu booting? */

extern unsigned long init_stack; /* defined in head.S */
extern unsigned long global_page_table;

extern unsigned char smp_boot_start[];
extern unsigned char smp_boot_end[];
unsigned char *smp_boot_base;

unsigned char smp_processor_id(void)
{
    unsigned long offset;
    unsigned char ret;

    offset = offsetof(struct cpu_info, apicid);
    asm volatile("movb %%gs:(%1), %0" : "=r"(ret) : "r"(offset) : "memory");
    return ret;
}

proc_t *make_idle()
{
    proc_t *idle;

    idle = (proc_t *)__get_free_pages(GFP_KERNEL, KERNEL_STACK_ORDER);
    idle->flags |= PF_IDLE;
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

    global_page_table = to_phy(kinfo.global_pgd_start);

    for (apicid = 0; apicid < nr_cpu; apicid++) {
        if (apicid == boot_apic_id)
            continue;
        current_boot_cpu = apicid;
        do_smp_boot(apicid);
    }
}

void smp_boot_main()
{
    printk("cpu%d boot initing...\n", current_boot_cpu);
    cpu_init();
    smp_irq_init();
    lapic_init();
    printk("cpu%d init finished\n", current_boot_cpu);
    smp_booting = 0;
    enable_interrupt();
    cpu_idle();
}
