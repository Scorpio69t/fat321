
.section .data

.code16

#include <boot/smp.h>

.global smp_boot_start
smp_boot_start:
base = .
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %es

	cli
	lidt	idt_ptr	- base
	lgdt 	gdt_ptr - base

	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es

	/* enable PE */
	movl	$1, %eax
	movl	%eax, %cr0

	xorl	%ebx, %ebx
	ljmpl	$0x08, $0x00100000  /* jmp to _start */

gdt_ptr:
	.short gdt32_end - gdt32
	.long  gdt32 - base + SMP_BOOT_BASE
idt_ptr:
	.short	0
	.long 	0

.align 64
gdt32:
	.quad 0x0000000000000000
	.quad 0x00cf9a000000ffff /* code */
	.quad 0x00cf92000000ffff /* data */
	.quad 0x0020980000000000 /* 64bit code */
	.quad 0x0000920000000000 /* 64bit data */
gdt32_end:

.global smp_boot_end
smp_boot_end:
