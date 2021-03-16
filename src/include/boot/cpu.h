#ifndef _BOOT_CPU_H_
#define _BOOT_CPU_H_

#include <feng/types.h>

#define load_tss(tss_desc) asm volatile("ltr %%ax" ::"a"((uint16)tss_desc))
#define load_ldt(ldt_desc) asm volatile("lldt %%ax" ::"a"((uint16)ldt_desc))

/* gdt_table and gdt_end are defiend in head.S */
extern uint8 gdt_table[];
extern uint8 gdt_end[];

struct gdtr_struct {
    uint16 len;
    uint64 base;
} __attribute__((packed));

#define IO_BITMAP_SIZE 16
struct tss_struct {
    uint32 reserved1;
    uint64 rsp0;
    uint64 rsp1;
    uint64 rsp2;
    uint64 reserved2;
    uint64 ist[7];
    uint32 reserved3;
    uint32 reserved4;
    uint16 reserved5;
    uint16 io_map_base;
    uint32 io_bitmap[IO_BITMAP_SIZE];
} __attribute__((packed));

#define IO_BITMAP_BASE offsetof(struct tss_struct, io_bitmap)

struct desc_struct {
    u16      limit0;
    u16      base0;
    unsigned base1 : 8, type : 4, s : 1, dpl : 2, p : 1;
    unsigned limit : 4, avl : 1, l : 1, d : 1, g : 1, base2 : 8;
} __attribute__((packed));

/* tss descriptor */
struct tss_desc_struct {
    uint16 limit0;
    uint16 base0;
    uint16 base1 : 8, type : 4, zero0 : 1, dpl : 2, p : 1;
    uint16 limit1 : 4, zero1 : 3, g : 1, base2 : 8;
    uint32 base3;
    uint32 zero2;
} __attribute__((packed));

/* 门描述符 */
struct gate_struct {
    uint16 offset0;
    uint16 segment;
    uint16 ist : 3, zero0 : 5, type : 4, zero1 : 1, dpl : 2, p : 1;
    u16    offset1;
    u32    offset2;
    u32    zero2;
} __attribute__((packed));

/**
 * 被中断时压入内核栈的寄存器, 按照压栈的顺序定义
 */
struct pt_regs {
    long ebx;
    long ecx;
    long edx;
    long esi;
    long edi;
    long ebp;
    long eax;
    int  ds;
    int  es;
    int  fs;
    int  gs;
    long orig_eax;
    long eip;
    long cs;
    long eflags;
    long esp;
    long ss;
} __attribute__((packed));

/**
 * 进程的cpu上下文信息
 */
struct thread_struct {
    unsigned long esp0; /* 内核栈基址 */
    unsigned long esp;  /* 内核栈当前栈指针 */
    unsigned long eip;  /* 内核态当前代码指针 */

    unsigned long cr2; /* cr2控制寄存器 */
};

/*
 * GDT的定义，固定为256个，只初始化了前6个
 * gdt[] 定义在 cpu.c
 */
#define GDT_SIZE 256
extern struct desc_struct gdt[];

/*
 * LDT的定义，固定为32个
 * ldt[] 定义在 cpu.c
 */
#define LDT_SIZE 32
extern struct desc_struct ldt[];

extern struct tss_struct tss;

void cpu_init(void);

/* cpu特权级 */

#define RING0 0
#define RING1 1
#define RING2 2
#define RING3 3

/* 选择子类型值说明 */
#define SA_RPL_MASK 0xFFFC
#define SA_RPL0     0
#define SA_RPL1     1
#define SA_RPL2     2
#define SA_RPL3     3

#define SA_TI_MASK 0xFFFB
#define SA_TIG     0
#define SA_TIL     4

#define USER_CODE_SELECTOR ((0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | SA_RPL3)
#define USER_DATA_SELECTOR ((8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | SA_RPL3)

/* gdt描述符选择子 */
#define SELECTOR_DUMMY   0          /* 空描述符 */
#define SELECTOR_FLAT_C  0x08       /* ring0代码段 */
#define SELECTOR_FLAT_RW 0x10       /* ring0数据段 */
#define SELECTOR_VIDEO   (0x18 + 3) /* ring3屏幕显示段 */
#define SELECTOR_TSS     0x20       /* TSS */
#define SELECTOR_LDT     0x28       /* LDT */

/* ldt描述符选择子 */
#define LDT_SELECTOR_FLAT_C  (0x00 + 3) /* ring3代码段 */
#define LDT_SELECTOR_FLAT_RW (0x08 + 3) /* ring3数据段 */

/* 描述符类型值说明 */
#define DA_32       0x4000 /* 32 位段				 */
#define DA_LIMIT_4K 0x8000 /* 段界限粒度为 4K 字节	   */
#define DA_DPL0     0x00   /* DPL = 0			    */
#define DA_DPL1     0x20   /* DPL = 1			    */
#define DA_DPL2     0x40   /* DPL = 2				*/
#define DA_DPL3     0x60   /* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define DA_DR   0x90 /* 存在的只读数据段类型值		    */
#define DA_DRW  0x92 /* 存在的可读写数据段属性值		    */
#define DA_DRWA 0x93 /* 存在的已访问可读写数据段类型值	 */
#define DA_C    0x98 /* 存在的只执行代码段属性值		    */
#define DA_CR   0x9A /* 存在的可执行可读代码段属性值		 */
#define DA_CCO  0x9C /* 存在的只执行一致代码段属性值		 */
#define DA_CCOR 0x9E /* 存在的可执行可读一致代码段属性值	  */
/* 系统段描述符类型值说明 */
#define DA_LDT      0x82 /* 局部描述符表段类型值			*/
#define DA_TaskGate 0x85 /* 任务门类型值				    */
#define DA_386TSS   0x89 /* 可用 386 任务状态段类型值    */
#define DA_386CGate 0x8C /* 386 调用门类型值			    */
#define DA_386IGate 0x8E /* 386 中断门类型值			    */
#define DA_386TGate 0x8F /* 386 陷阱门类型值			    */

#define sel_to_ind(s) (s >> 3)

/*
 * 根据选择子s得到t中描述符的地址，适用于gdt，ldt
 */
#define get_desc(t, s) (&t[sel_to_ind(s)])

/*
 * 根据段描述符求段的基地址
 */
static inline u32 seg_to_phys(u16 seg)
{
    // struct desc_struct *p_dest = get_desc(gdt, seg);
    // return (p_dest->base_high << 24 | p_dest->base_mid << 16 | p_dest->base_low);
    return 0;
}

/*
 * 线性地址求物理地址
 */
static inline u32 vir_to_phys(u32 base, u32 offset)
{
    return (base + offset);
}

#endif
