/*
 * cpu相关
 */

#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/smp.h>
#include <config.h>
#include <kernel/sched.h>
#include <kernel/string.h>
#include <kernel/types.h>

struct tss_struct init_tss[NR_CPUS];
struct cpu_info   cpu_info[NR_CPUS];

struct desc_struct ldt[LDT_SIZE];

extern unsigned long init_stack; /* defined in head.S */

static void setup_tss_desc(struct tss_desc_struct *desc, uint64 addr, uint32 size)
{
    memset(desc, 0x00, sizeof(struct tss_desc_struct));
    desc->limit0 = size & 0xffff;
    desc->base0 = addr & 0xffff;
    desc->base1 = (addr >> 16) & 0xffff;
    desc->type = 0x9;
    desc->p = 1;
    desc->limit1 = (size >> 16) & 0xf;
    desc->base2 = (addr >> 24) & 0xff;
    desc->base3 = addr >> 32;
}

void cpu_init(void)
{
    struct tss_struct *     tss;
    struct tss_desc_struct *tss_desc;
    unsigned long           tss_desc_index;

    tss = &init_tss[current_boot_cpu];
    tss->rsp0 = (uint64)init_stack;
    memset(tss->io_bitmap, 0xff, sizeof(tss->io_bitmap));
    tss->io_map_base = IO_BITMAP_BASE;

    tss_desc = (struct tss_desc_struct *)gdt_tss + current_boot_cpu;
    setup_tss_desc(tss_desc, (uint64)tss, sizeof(struct tss_struct));
    tss_desc_index = (unsigned long)tss_desc - (unsigned long)gdt_table;
    load_tss(tss_desc_index);

    wrmsr(MSR_KERNEL_GS_BASE, 0);
    wrmsr(MSR_FS_BASE, 0);
    wrmsr(MSR_GS_BASE, &cpu_info[current_boot_cpu]);
}

void cpu_idle(void)
{
    while (1) {
        asm volatile("hlt");
    }
}

void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
    asm volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "0"(op));
}
