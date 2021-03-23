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
    uint64 r15;
    uint64 r14;
    uint64 r13;
    uint64 r12;
    uint64 rbp;
    uint64 rbx;
    uint64 r11;
    uint64 r10;
    uint64 r9;
    uint64 r8;
    uint64 rax;
    uint64 rcx;
    uint64 rdx;
    uint64 rsi;
    uint64 rdi;
    uint64 orig_rax;
    uint64 rip;
    uint64 cs;
    uint64 eflags;
    uint64 rsp;
    uint64 ss;
} __attribute__((packed));

/**
 * 进程的cpu上下文信息
 */
struct thread_struct {
    uint64 rsp0; /* 内核栈基址 */
    uint64 rsp;  /* 内核栈当前栈指针 */
    uint64 rip;  /* 内核态当前代码指针 */

    unsigned long cr2; /* cr2控制寄存器 */
};

struct cpu_info {
    uint64 kernelstack; /* kernel stack */
    uint64 oldrsp;      /* 暂存syscall过来的用户栈 */
};

#define MSR_FS_BASE        0xc0000100
#define MSR_GS_BASE        0xc0000101
#define MSR_KERNEL_GS_BASE 0xc0000102

#define wrmsr(msr, val) asm volatile("wrmsr" ::"c"(msr), "a"(((uint64)val) & 0xffffffff), "d"(((uint64)val) >> 32))

#define rdmsr(msr, val)                                              \
    do {                                                             \
        uint64 __rax, __rdx;                                         \
        asm volatile("rdmsr" : "=a"(__rax), "=d"(__rdx) : "c"(msr)); \
        val = __rax | (__rdx << 32);                                 \
    } while (0);

/*
 * LDT的定义，固定为32个
 * ldt[] 定义在 cpu.c
 */
#define LDT_SIZE 32
extern struct desc_struct ldt[];

extern struct tss_struct init_tss;

void cpu_init(void);

/* cpu特权级 */
#define RING0 0
#define RING1 1
#define RING2 2
#define RING3 3

#endif
