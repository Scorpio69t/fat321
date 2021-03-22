/*
 * cpu相关
 */

#include <boot/boot.h>
#include <boot/cpu.h>
#include <feng/sched.h>
#include <feng/string.h>
#include <feng/types.h>

static struct tss_struct init_tss;
static struct cpu_info cpu_info;
struct desc_struct ldt[LDT_SIZE];

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
    struct tss_struct *tss = &init_tss;
    tss->rsp0 = (uint64)&init_task_union + KERNEL_STACK_SIZE - 8;
    memset(tss->io_bitmap, 0xff, sizeof(tss->io_bitmap));
    tss->io_map_base = IO_BITMAP_BASE;

    setup_tss_desc((void *)gdt_table + TSS_DESC, (uint64)tss, sizeof(struct tss_struct));
    load_tss(TSS_DESC);

    wrmsr(MSR_KERNEL_GS_BASE, &cpu_info);
    wrmsr(MSR_FS_BASE, 0);
    wrmsr(MSR_GS_BASE, 0);
}
