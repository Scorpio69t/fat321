/*
 * cpu相关
 */

#include <boot/boot.h>
#include <boot/cpu.h>
#include <feng/sched.h>
#include <feng/string.h>
#include <feng/types.h>

struct tss_struct init_tss;

struct desc_struct ldt[LDT_SIZE];

static inline void init_descriptor(struct desc_struct *p_desc, u32 base, u32 limit, u16 attribute) {}

static inline void cpu_ldt_init(void)
{
    /*
     * ring3代码段描述符，寻址空间0x00000000 - 0xffffffff(4GB)，可读可执行
     */
    init_descriptor(get_desc(ldt, LDT_SELECTOR_FLAT_C), 0x00, 0xfffff, DA_LIMIT_4K | DA_32 | DA_CR | DA_DPL3);
    /*
     * ring3数据段描述符，寻址空间0x00000000 - 0xffffffff(4GB), 可读可写
     */
    init_descriptor(get_desc(ldt, LDT_SELECTOR_FLAT_RW), 0x00, 0xfffff, DA_LIMIT_4K | DA_32 | DA_DRW | DA_DPL3);

    /* 在gdt中添加ldt描述符 */
    init_descriptor(get_desc(gdt, SELECTOR_LDT), vir_to_phys(seg_to_phys(SELECTOR_FLAT_RW), (u32)&ldt),
                    LDT_SIZE * sizeof(struct desc_struct) - 1, DA_LDT);

    load_ldt(SELECTOR_LDT);
}

static inline void cpu_tss_init(void)
{
    // /* 在gdt中添加tss描述符 */
    // init_descriptor(get_desc(gdt, SELECTOR_TSS), vir_to_phys(seg_to_phys(SELECTOR_FLAT_RW), (u32)&tss),
    //                 sizeof(struct tss_struct) - 1, DA_386TSS);
    // /*
    //  * 填充tss，初始化ring0栈
    //  */
    // tss.ss0 = SELECTOR_FLAT_RW;
    // tss.iobase = sizeof(tss);

    // load_tss(SELECTOR_TSS);
}

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
}
