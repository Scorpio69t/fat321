/*
 * cpu相关
 */

#include <alphaz/type.h>
#include <asm/cpu.h>

struct tss_struct tss;
struct desc_struct gdt[GDT_SIZE];
struct desc_struct ldt[LDT_SIZE];

static inline void init_descriptor(struct desc_struct *p_desc, u32 base,
                                    u32 limit, u16 attribute)
{
    p_desc->limit_low = limit & 0x0FFFF;
    p_desc->base_low = base & 0x0FFFF;
    p_desc->base_mid = (base >> 16) & 0x0FF;
    p_desc->attr1 = attribute & 0xFF;
    p_desc->limit_high_attr2 = ((limit >> 16) & 0x0F) | \
                                ((attribute >> 8) & 0xF0);
    p_desc->base_high = (base >> 24) & 0x0FF;
}

static inline void load_gdt(struct gdtr_struct gdtr)
{
    asm volatile(
        "lgdt %0\n\t"
        "jmp %1,$flash_label\n\t"
        /* 刷新寄存器 */
        "flash_label:"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%ss\n\t"
        "mov %%bx, %%gs\n\t"
        :
        :"m"(gdtr), "i"(SELECTOR_FLAT_C), "a"(SELECTOR_FLAT_RW),
         "b"(SELECTOR_VIDEO));
}

static inline void load_ldt(u16 ldt_sel)
{
    asm volatile("lldt %%ax"::"a"(ldt_sel));
}

static inline void load_tss(u16 tss_sel)
{
    asm volatile("ltr %%ax"::"a"(tss_sel));
}

/*
 * 当loader加载完内核后，loader便没有了作用，其所占用的内存将会被kernel覆盖，其中gdt将
 * 重新在此定义，并在内核中维护
 * 该处的代码段数据段和屏幕显示段的描述符与loader相同，不再重新初始化段寄存器
 */
static void cpu_gdt_init(void)
{


    /*
     * gdt首个空描述符
     */
    init_descriptor(get_desc(gdt, SELECTOR_DUMMY), 0x00, 0x00, 0x00);

    /*
     * ring0代码段描述符，寻址空间0x00000000 - 0xffffffff(4GB)，可读可执行
     */
    init_descriptor(get_desc(gdt, SELECTOR_FLAT_C), 0x00,
                        0xfffff, DA_LIMIT_4K | DA_32 | DA_CR);

    /*
     * ring0数据段描述符，寻址空间0x00000000 - 0xffffffff(4GB), 可读可写
     */
    init_descriptor(get_desc(gdt, SELECTOR_FLAT_RW), 0x00,
                        0xfffff, DA_LIMIT_4K | DA_32 | DA_DRW);

    /*
     * ring3显示段描述符，寻址空间0x000b8000 - 0x000c7fff, 可读可写
     * 用于在屏幕上显示
     */
    init_descriptor(get_desc(gdt, SELECTOR_VIDEO), 0xb8000,
                        0xffff, DA_DRW | DA_DPL3);

    /*
     * 初始化gdt的基址和界限，前两字节为界限，后四个字节位基址
     */
    static struct gdtr_struct gdtr;
    gdtr.len = GDT_SIZE * sizeof(struct desc_struct) - 1;
    gdtr.base = (u32)&gdt;

    load_gdt(gdtr);
}

static inline void cpu_ldt_init(void)
{
    /*
     * ring3代码段描述符，寻址空间0x00000000 - 0xffffffff(4GB)，可读可执行
     */
    init_descriptor(get_desc(ldt, LDT_SELECTOR_FLAT_C), 0x00,
                    0xfffff, DA_LIMIT_4K | DA_32 | DA_CR | DA_DPL3);
    /*
     * ring3数据段描述符，寻址空间0x00000000 - 0xffffffff(4GB), 可读可写
     */
    init_descriptor(get_desc(ldt, LDT_SELECTOR_FLAT_RW), 0x00,
                    0xfffff, DA_LIMIT_4K | DA_32 | DA_DRW | DA_DPL3);

    /* 在gdt中添加ldt描述符 */
    init_descriptor(get_desc(gdt, SELECTOR_LDT),
                    vir_to_phys(seg_to_phys(SELECTOR_FLAT_RW), (u32)&ldt),
                    LDT_SIZE * sizeof(struct desc_struct) - 1, DA_LDT);


    load_ldt(SELECTOR_LDT);
}

static inline void cpu_tss_init(void)
{
    /* 在gdt中添加tss描述符 */
    init_descriptor(get_desc(gdt, SELECTOR_TSS),
                    vir_to_phys(seg_to_phys(SELECTOR_FLAT_RW),(u32)&tss),
                    sizeof(struct tss_struct) - 1, DA_386TSS);
    /*
     * 填充tss，初始化ring0栈
     */
    tss.ss0 = SELECTOR_FLAT_RW;
    tss.iobase = sizeof(tss);

    load_tss(SELECTOR_TSS);
}

void cpu_init(void)
{
    cpu_gdt_init();
    cpu_ldt_init();
    cpu_tss_init();
}
