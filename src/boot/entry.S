/*
 * 中断入口
 */
#include <kernel/sched.h>
#include <kernel/fork.h>
#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/apic.h>

.section .text

.macro SAVE_ALL
	cld
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rax
	pushq   %r8
	pushq   %r9
	pushq   %r10
	pushq   %r11
	pushq   %rbx
	pushq   %rbp
	pushq   %r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
.endm

.macro RESTOR_ALL
	popq    %r15
	popq    %r14
	popq    %r13
	popq    %r12
	popq    %rbp
	popq    %rbx
	popq    %r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rax
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
.endm

.macro 	CURRENT reg
	movq	$-KERNEL_STACK_SIZE, \reg
	andq	%rsp, \reg
.endm

.global ret_from_fork
ret_from_fork:
	cli
	RESTOR_ALL
	testq $3,16(%rsp)	/* test CS RPL */
	jz 1f
	swapgs
1:	addq 	$8, %rsp	/* skip orgi_rax */
	xorq	%rax, %rax
	movq	$0, 0x8(%rsi) 	/* The return value of the child process in message struct */
	iretq

.global exec_ret
exec_ret:
	cli
	RESTOR_ALL
	testq $3,16(%rsp)	/* test CS RPL */
	jz 1f
	swapgs
1:	addq	$8, %rsp
	iretq

.global apic_timer
apic_timer:
	pushq $0x20
	testq $3,16(%rsp)	/* test CS RPL */
	jz 1f
	swapgs
1:	SAVE_ALL
	movq	%rsp, %rdi
	call    do_timer
	jmp     ret_from_int

.macro HWINTR irq
.global	hwint\irq
hwint\irq:
	pushq	$\irq
	jmp	hwintr
.endm
	HWINTR	0x21
	HWINTR	0x22
	HWINTR	0x23
	HWINTR	0x24
	HWINTR	0x25
	HWINTR 	0x26
	HWINTR	0x27
	HWINTR	0x28
	HWINTR	0x29
	HWINTR	0x2a
	HWINTR	0x2b
	HWINTR	0x2c
	HWINTR	0x2d
	HWINTR	0x2e
	HWINTR	0x2f
hwintr:
	testq $3,16(%rsp)	/* test CS RPL */
	jz 1f
	swapgs
1:	SAVE_ALL
	movq	%rsp, %rdi
	call    do_IRQ
	jmp     ret_from_int

ret_from_int:
	movq	$(KERNEL_OFFSET + 0xfee000b0), %rax
	movl	$0, (%rax)	/* send EOI to local APIC */
	CURRENT %rbx
	testl	$NEED_SCHEDULE, PROC_FLAGS(%rbx)
	jnz	resched
	jmp     intr_ret
resched:
	call    schedule
intr_ret:
  	RESTOR_ALL
	testq $3, 16(%rsp)	/* test CS RPL */
	jz 1f
	swapgs
1:    	addq    $8, %rsp
    	iretq

/* 中断和异常 -- 异常 */
.global divide_error
divide_error:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$0
	jmp 	exception

.global single_step_exception
single_step_exception:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$1
	jmp 	exception

.global nmi
nmi:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$2
	jmp	exception

.global breakpoint_exception
breakpoint_exception:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$3
	jmp	    exception

.global overflow
overflow:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$4
	jmp	    exception

.global bounds_check
bounds_check:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$5
	jmp	    exception

.global inval_opcode
inval_opcode:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$6
	jmp	    exception

.global copr_not_available
copr_not_available:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$7
	jmp	    exception

.global double_fault
double_fault:
	pushq	$8
	jmp	    exception

.global copr_seg_overrun
copr_seg_overrun:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$9
	jmp	    exception

.global inval_tss
inval_tss:
	pushq	$10
	jmp	    exception

.global segment_not_present
segment_not_present:
	pushq	$11
	jmp	    exception

.global stack_exception
stack_exception:
	pushq	$12
	jmp	    exception

.global general_protection
general_protection:
	pushq	$13
	jmp	    exception

.global page_fault
page_fault:
	pushq	$14
	jmp	    exception

.global copr_error
copr_error:
	movq 	$0xffffffff, %rax
	pushq	%rax
	pushq	$16
	jmp	    exception

exception:
	SAVE_ALL
	movq    %rsp, %rdi
	cli
	call	do_IRQ
	hlt

.global spurious_intr
spurious_intr:
	pushq	$APIC_IRQ_SPURIOUS
	SAVE_ALL
	cli
	movq	%rsp, %rdi
	call 	spurious_handler
	hlt

.global apic_error
apic_error:
	pushq $APIC_IRQ_ERROR
	SAVE_ALL
	cli
	movq	%rsp, %rdi
	movq	$APIC_IRQ_ERROR, %rsi
	call	exception_handler
	hlt

/* 系统调用入口 int 0x80
 * rax中为功能号
 */
.global system_call
system_call:
	pushq   %rax    /* 压入功能号 */
	testq 	$3, 16(%rsp)	/* test CS RPL */
	jz 	1f
	swapgs
1:	SAVE_ALL
	cmpl	$3, %eax
	je	2f
	movq	%rsp, %rdi
	call	pre_syscall
2:	leaq	syscall_table(%rip), %rbx
	movq	SF_ORIG(%rsp), %rax
	movq	(%rbx,%rax,8), %rax
	movq	%rsp, %rdi
	call	*%rax
	movq	%rax, SF_RAX(%rsp)
	RESTOR_ALL
	testq	$3, 16(%rsp)
	jz	3f
	swapgs
3:	addq    $8, %rsp
    	iretq

.global syscall_table
syscall_table:
	.quad  sys_send
	.quad  sys_recv
	.quad  sys_sendrecv
	.quad  sys_debug
syscall_table_end:

syscall_table_len:
	.quad	syscall_table_end - syscall_table
