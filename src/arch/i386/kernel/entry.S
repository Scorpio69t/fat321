/*
 * 中断入口
 */

#include <alphaz/linkage.h>
#include <alphaz/sched.h>

.section .text

#define SAVE_ALL    \
    cld;            \
    pushl   %gs;    \
    pushl   %fs;    \
    pushl   %es;    \
    pushl   %ds;    \
    pushl   %eax;   \
    pushl   %ebp;   \
    pushl   %edi;   \
    pushl   %esi;   \
    pushl   %edx;   \
    pushl   %ecx;   \
    pushl   %ebx

#define RESTOR_ALL  \
    popl    %ebx;   \
    popl    %ecx;   \
    popl    %edx;   \
    popl    %esi;   \
    popl    %edi;   \
    popl    %ebp;   \
    popl    %eax;   \
    popl    %ds;    \
    popl    %es;    \
    popl    %fs;    \
    popl    %gs

PT_EAX = 0x18

TASK_STATE  = 0x00
TASK_FLAGS  = 0x04

#define GET_CURRENT(reg)	\
	movl	$-KERNEL_STACK_SIZE, reg;	\
	andl	%esp, reg

#define INTERRUPT_RETURN     \
    addl    $4, %esp;        \
    iretl

#define PUSH_PT_ARGS    \
    movl    %esp, %eax; \
    pushl   %eax

#define POP_PT_ARGS     \
    addl    $4, %esp

/* could not use stack */
#define RESET_8259A_CHIP    \
    movb    $0x20, %al;     \
    outb    %al, $0x20;     \
    outb    %al, $0xa0

/* could not use stack */
/* 这里传入的是中断号，相应的引脚号要减去0x20 */
#define CLOSE_8259A_PIN(irq)    \
    movb    $8, %al;            \
    cmpb    $(irq-0x20), %al;   \
    jle     1f;                 \
    inb     $0x21, %al;         \
    bts     $(irq-0x20), %ax;   \
    outb    %al, $0x21;         \
    jmp     2f;                 \
  1:inb     $0xa1, %al;         \
    bts     $(irq-0x28), %ax;   \
    outb    %al, $0xa1;         \
  2: ;

/* could not use stack */
#define OPEN_8259A_PIN(irq)     \
    movb    $8, %al;            \
    cmpb    $(irq-0x20), %al;   \
    jle     1f;                 \
    inb     $0x21, %al;         \
    btr     $(irq-0x20),  %ax;  \
    outb    %al, $0x21;         \
    jmp     2f;                 \
  1:inb     $0xa1, %al;         \
    btr     $(irq-0x28), %ax;   \
    outb    %al, $0xa1;         \
  2: ;

#define RING0_HW_IRQ_FREAM(irq)    \
    pushl $irq;                 \
    SAVE_ALL;                   \
    CLOSE_8259A_PIN(irq);       \
    RESET_8259A_CHIP;           \
    sti;                        \
    PUSH_PT_ARGS;               \
    call    do_IRQ;             \
    POP_PT_ARGS;                \
    OPEN_8259A_PIN(irq);        \
    jmp     ret_from_int


ENTRY(ret_from_int)
    GET_CURRENT(%ebx)
    testl    $NEED_SCHEDULE, TASK_FLAGS(%ebx)
    jnz     re_sched
    jmp     int_ret
re_sched:
    call    schedule
int_ret:
    RESTOR_ALL
    INTERRUPT_RETURN
END(ret_from_int)


ENTRY(ret_from_fork)
    cli
    RESTOR_ALL
    INTERRUPT_RETURN
END(ret_from_fork)

ENTRY(hwint0x20) RING0_HW_IRQ_FREAM(0x20); END(hwint0x20)
ENTRY(hwint0x21) RING0_HW_IRQ_FREAM(0x21); END(hwint0x21)
ENTRY(hwint0x22) RING0_HW_IRQ_FREAM(0x22); END(hwint0x22)
ENTRY(hwint0x23) RING0_HW_IRQ_FREAM(0x23); END(hwint0x23)
ENTRY(hwint0x24) RING0_HW_IRQ_FREAM(0x24); END(hwint0x24)
ENTRY(hwint0x25) RING0_HW_IRQ_FREAM(0x25); END(hwint0x25)
ENTRY(hwint0x26) RING0_HW_IRQ_FREAM(0x26); END(hwint0x26)
ENTRY(hwint0x27) RING0_HW_IRQ_FREAM(0x27); END(hwint0x27)
ENTRY(hwint0x28) RING0_HW_IRQ_FREAM(0x28); END(hwint0x28)
ENTRY(hwint0x29) RING0_HW_IRQ_FREAM(0x29); END(hwint0x29)
ENTRY(hwint0x2a) RING0_HW_IRQ_FREAM(0x2a); END(hwint0x2a)
ENTRY(hwint0x2b) RING0_HW_IRQ_FREAM(0x2b); END(hwint0x2b)
ENTRY(hwint0x2c) RING0_HW_IRQ_FREAM(0x2c); END(hwint0x2c)
ENTRY(hwint0x2d) RING0_HW_IRQ_FREAM(0x2d); END(hwint0x2d)
ENTRY(hwint0x2e) RING0_HW_IRQ_FREAM(0x2e); END(hwint0x2e)
ENTRY(hwint0x2f) RING0_HW_IRQ_FREAM(0x2f); END(hwint0x2f)

/* 中断和异常 -- 异常 */
ENTRY(divide_error)
	pushl	$0xffffffff
	pushl	$0
	jmp 	exception
END(divide_error)

ENTRY(single_step_exception)
	pushl	$0xffffffff
	pushl	$1
	jmp 	exception
END(single_step_exception)

ENTRY(nmi)
	pushl	$0xffffffff
	pushl	$2
	jmp	    exception
END(nmi)

ENTRY(breakpoint_exception)
	pushl	$0xffffffff
	pushl	$3
	jmp	    exception
END(breakpoint_exception)

ENTRY(overflow)
	pushl	$0xffffffff
	pushl	$4
	jmp	    exception
END(overflow)

ENTRY(bounds_check)
	pushl	$0xffffffff
	pushl	$5
	jmp	    exception
END(bounds_check)

ENTRY(inval_opcode)
	pushl	$0xffffffff
	pushl	$6
	jmp	    exception
END(inval_opcode)

ENTRY(copr_not_available)
	pushl	$0xffffffff
	pushl	$7
	jmp	    exception
END(copr_not_available)

ENTRY(double_fault)
	pushl	$8
	jmp	    exception
END(double_fault)

ENTRY(copr_seg_overrun)
	pushl	$0xffffffff
	pushl	$9
	jmp	    exception
END(copr_seg_overrun)

ENTRY(inval_tss)
	pushl	$10
	jmp	    exception
END(inval_tss)

ENTRY(segment_not_present)
	pushl	$11
	jmp	    exception
END(segment_not_present)

ENTRY(stack_exception)
	pushl	$12
	jmp	    exception
END(stack_exception)

ENTRY(general_protection)
	pushl	$13
	jmp	    exception
END(general_protection)

ENTRY(page_fault)
	pushl	$14
	jmp	    exception
END(page_fault)

ENTRY(copr_error)
	pushl	$0xffffffff
	pushl	$16
	jmp	    exception
END(copr_error)

exception:
    SAVE_ALL
    PUSH_PT_ARGS
    cli
	call	do_IRQ
    hlt

/* 系统调用入口 int 0x80
 * eax中为功能号
 */
ENTRY(system_call)
    pushl   %eax    /* 压入功能号 */
	SAVE_ALL
    movl    PT_EAX(%esp), %eax
    call    *sys_call_table(,%eax,4)
    movl    %eax, PT_EAX(%esp)
    RESTOR_ALL
    INTERRUPT_RETURN
END(system_call)

ENTRY(kernel_thread_ret)
    popl    %ebx
    popl    %ecx
    popl    %edx
    popl    %esi
    popl    %edi
    popl    %ebp
    popl    %eax
    popl    %ds
    popl    %es
    popl    %fs
    popl    %gs
    call    *%ebx
    pushl   %eax
    call    sys_exit
END(kernel_thread_ret)
