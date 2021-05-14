/*
 * 内核初始化入口
 */

#include <boot/apic.h>
#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/mptable.h>
#include <boot/smp.h>
#include <kernel/bugs.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/types.h>

volatile int boot_finished = 0;

kinfo_t kinfo;

void kernel_main(void *boot_info)
{
    printk("booting...\n");
    memset(&kinfo, 0x00, sizeof(kinfo_t));

    extern char _kernel_start, _kernel_end;
    kinfo.kernrl_start = (uint64)&_kernel_start;
    kinfo.kernel_end = (uint64)&_kernel_end;
    printk("kernel start: 0x%016llx, end: 0x%016llx\n", kinfo.kernrl_start, kinfo.kernel_end);

    printk("scaning boot info...\n");
    scan_boot_info((uint64)boot_info);

    printk("Initializing cpu...\n");
    cpu_init();

    printk("Initializing interrupt...\n");
    irq_init();

    printk("Initializing memory management...\n");
    mm_init();

    printk("Initializing proc...\n");
    proc_init();

    mp_init();
    apic_init();
    smp_init();

    printk("kernel_main\n");

    enable_interrupt();
    boot_finished = 1;
    cpu_idle();
}
