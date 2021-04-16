/*
 * 中断入口
 */

#include <kernel/linkage.h>
#include <kernel/sched.h>
#include <kernel/fork.h>
#include <boot/boot.h>
#include <boot/cpu.h>

.section .text

#define SAVE_ALL    \
	cld;            \
	pushq	%rdi;	\
	pushq	%rsi;	\
	pushq	%rdx;	\
	pushq	%rcx;	\
	pushq	%rax;	\
	pushq   %r8;   	\
	pushq   %r9;   	\
	pushq   %r10;   \
	pushq   %r11;   \
	pushq   %rbx;   \
	pushq   %rbp;   \
	pushq   %r12;	\
	pushq	%r13;	\
	pushq	%r14;	\
	pushq	%r15

#define RESTOR_ALL  \
	popq    %r15;   \
	popq    %r14;   \
	popq    %r13;   \
	popq    %r12;   \
	popq    %rbp;   \
	popq    %rbx;   \
	popq    %r11;   \
	popq	%r10;	\
	popq	%r9;	\
	popq	%r8;	\
	popq	%rax;	\
	popq	%rcx;	\
	popq	%rdx;	\
	popq	%rsi;	\
	popq	%rdi

#define GET_CURRENT(reg)	\
	movq	$-KERNEL_STACK_SIZE, reg;	\
	andq	%rsp, reg

#define INTERRUPT_RETURN     \
    addq    $8, %rsp;        \
    iretq

#define PUSH_PT_ARGS    \
    movq    %rsp, %rdi

#define POP_PT_ARGS

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
	pushq $irq;                 \
	testq $3,16(%rsp);	/* test CS RPL */	\
	je 1f; 				\
	swapgs;				\
1:	SAVE_ALL;                   \
	CLOSE_8259A_PIN(irq);       \
	RESET_8259A_CHIP;           \
	sti;                        \
	PUSH_PT_ARGS;               \
	call    do_IRQ;             \
	POP_PT_ARGS;                \
	OPEN_8259A_PIN(irq);        \
	jmp     ret_from_int


ret_from_int:
	GET_CURRENT(%rbx)
	testl	$NEED_SCHEDULE, PROC_FLAGS(%rbx)
	jnz	re_sched
	jmp     int_ret
	re_sched:
	call    schedule
	int_ret:
	testq $3, SF_CS(%rsp)
	je 1f
	swapgs
1:  	RESTOR_ALL
    	INTERRUPT_RETURN

.global ret_from_fork
ret_from_fork:
	cli
	RESTOR_ALL
	addq 	$8, %rsp	/* skip orgi_rax */
	xorq	%rax, %rax
	movq	$0, 0x8(%rsi) 	/* The return value of the child process in message struct */
	iretq

.global exec_ret
exec_ret:
	cli
	RESTOR_ALL
	addq	$8, %rsp
	iretq

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
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$0
	jmp 	exception
END(divide_error)

ENTRY(single_step_exception)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$1
	jmp 	exception
END(single_step_exception)

ENTRY(nmi)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$2
	jmp	exception
END(nmi)

ENTRY(breakpoint_exception)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$3
	jmp	    exception
END(breakpoint_exception)

ENTRY(overflow)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$4
	jmp	    exception
END(overflow)

ENTRY(bounds_check)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$5
	jmp	    exception
END(bounds_check)

ENTRY(inval_opcode)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$6
	jmp	    exception
END(inval_opcode)

ENTRY(copr_not_available)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$7
	jmp	    exception
END(copr_not_available)

ENTRY(double_fault)
	pushq	$8
	jmp	    exception
END(double_fault)

ENTRY(copr_seg_overrun)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$9
	jmp	    exception
END(copr_seg_overrun)

ENTRY(inval_tss)
	pushq	$10
	jmp	    exception
END(inval_tss)

ENTRY(segment_not_present)
	pushq	$11
	jmp	    exception
END(segment_not_present)

ENTRY(stack_exception)
	pushq	$12
	jmp	    exception
END(stack_exception)

ENTRY(general_protection)
	pushq	$13
	jmp	    exception
END(general_protection)

ENTRY(page_fault)
	pushq	$14
	jmp	    exception
END(page_fault)

ENTRY(copr_error)
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$16
	jmp	    exception
END(copr_error)

exception:
	SAVE_ALL
	PUSH_PT_ARGS
	cli
	call	do_IRQ
	hlt

/* 系统调用入口 int 0x80
 * rax中为功能号
 */
.global system_call
system_call:
	pushq   %rax    /* 压入功能号 */
	SAVE_ALL
	cmpl	$3, %eax
	je	1f
	movq	%rsp, %rdi
	call	pre_syscall
1:	leaq	syscall_table(%rip), %rbx
	movq	SF_ORIG(%rsp), %rax
	movq	(%rbx,%rax,8), %rax
	movq	%rsp, %rdi
	call	*%rax
	movq	%rax, SF_RAX(%rsp)
	RESTOR_ALL
	INTERRUPT_RETURN

kernel_process_child:
	movq	%rdi, %rax	/* kernle process function address */
	movq	%rsi, %rdi	/* function arguments */
	call	*%rax
	xorq	%rdi, %rdi	/* exit code 0 */
	call	do_exit

/*
 * C interface pid_t kernel_proc(int (*fn)(void), void *args, unsigned long flags)
 * fn in rdi, args in rsi and flags in rdx
 */
.global kernel_proc
kernel_proc:
	/* make fake frame for do_fork */
	xorq	%rax, %rax
	pushq	%rax			/* ss */
	pushq	%rax			/* rsp, will be set in do_fork */
	movq	$(1<<9), %rax
	pushq	%rax			/* eflags, enable interrupt */
	movq	$KERNEL_CODE_DESC, %rax
	pushq	%rax			/* cs */
	leaq	kernel_process_child(%rip), %rax
	pushq	%rax			/* rip */
	xorq	%rax, %rax
	pushq	%rax			/* orgi_rax */

	SAVE_ALL

	/* prepare for do_fork */
	movq	%rdx, %rdi 		/* clone_flags */
	movq	$-1, %rsi		/* start_stack */
	movq	%rsp, %rdx		/* regs */
	xorq	%rcx, %rcx		/* stack_size */
	call	do_fork

	movq	%rax, 0x50(%rsp)	/* save return value */
	RESTOR_ALL
	addq	$8*6, %rsp		/* skip fake frame */
	ret

.global syscall_table
syscall_table:
	.quad  sys_send
	.quad  sys_recv
	.quad  sys_sendrecv
	.quad  sys_debug
syscall_table_end:

syscall_table_len:
	.quad	syscall_table_end - syscall_table
