#include <boot/multiboot.h>
#include <boot/boot.h>
#include <kernel/linkage.h>
#include <kernel/sched.h>

/*
 * This file is the kernel head that will be loaded at the physical address
 * 0x100000 when booting.
 * In this file, we use fixed adresses to avoid 32bit relocations.
 */
	.section ".head.text"
	.code32

.global _start
_start:
	/* check multiboot2 magic */
	cmpl 	$MULTIBOOT2_BOOTLOADER_MAGIC, %eax
	je	magic_ok
	hlt
magic_ok:
	/* 32-bit physical address of the Multiboot2 information structure */
	movl	%ebx, 0x100f30
	/* check if cpu support extended function information */
	movl	$0x80000000, %eax
	cpuid
	cmpl 	$0x80000000, %eax
	jbe 	start_hlt		/* not support */

	/* check if cpu support long mode
	 * the information in CPUID.80000001h:EDX[29].LM
	 */
	movl	$0x80000001, %eax
	cpuid
	btl 	$29, %edx
	jnc	start_hlt

	/*
	 * switch to long mode
	 */

	/* enable PAE(Physical Address Extension) */
	xorl	%eax, %eax
	btsl	$5, %eax
	movl	%eax, %cr4

	/* setup page table */
	movl	$0x101000, %eax		/* the address of PML4E */
	movl	%eax, %cr3

	movl	$0xc0000080, %ecx
	rdmsr
	btsl	$8, %eax		/* enable LME */
	wrmsr

	movl	%cr0, %eax
	btsl	$0, %eax		/* enable PE, protected mode*/
	btsl	$31, %eax		/* enable PG */
	movl	%eax, %cr0
	jmp	compatibility_mode	/* flash page table */

compatibility_mode:
	/*
	 * Now we are in 32bit compatibility mode
	 */

	movl	$0x100f00, %eax		/* the address of pGDT32 */
	lgdt	(%eax)

	movl	$0x100f20, %eax		/* the address of long_jump_vector */
	ljmp 	*(%eax)

start_hlt:
	hlt

	.code64
	.org 0x200
real_long_mode:
	movq	init_stack(%rip), %rax
	movq	%rax, %rsp

	movq	$(0x100f40+KERNEL_OFFSET), %rax	/* the address of gdt_desc */
	lgdt	(%rax)

	movl	$0x10,	%eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %ss

	movq	kernel_main_entry(%rip), %rax
	jmp	*%rax

	hlt


.code32
.org 0xf00
pGDT32:					/* address 0x100f00 */
	.word	GDT32_end - GDT32
	.long	0x100f10		/* the address of GDT32 */

.org 0xf10
GDT32:
	.quad 0x0000000000000000	/* null descriptor */
	.quad 0x0020980000000000	/* code descriptor */
GDT32_end:


.org 0xf20
long_jump_vector:			/* address 0x100f20 */
	.long	0x100200		/* the address of real_long_mode */
	.word	0x08			/* code descriptor */

.org 0xf30
.global boot_info
boot_info:				/* address 0x100f30 */
	.long	0

.org 0xf40
.global	gdt_desc
gdt_desc:				/* address 0x100f40 */
	.word	gdt_end - gdt_table
	.quad	gdt_table

init_stack:
	.quad 	init_proc_union + KERNEL_STACK_SIZE - 8

kernel_main_entry:
	.quad	kernel_main

/*
 * We use 2Mbyte large pages provided by PAE mode
 */
.org 0x1000
/* page map level 4 table */
PML4E:						/* address 0x101000 */
	.quad	0x0000000000102007		/* for usr */
	.fill	255,8,0
	.quad	0x0000000000102007		/* for kernel */
	.fill	255,8,0

.org 0x2000
/* page directory pointer table */
PDPTE:
	.quad	0x0000000000103007
	.fill	511,8,0


.org 0x3000
/* page directory, total mapping 32Mb memory */
PDE:
	.quad	0x0000000000000087
	.quad	0x0000000000200087
	.quad	0x0000000000400087
	.quad	0x0000000000600087
	.quad	0x0000000000800087
	.quad	0x0000000000a00087
	.quad	0x0000000000c00087
	.quad	0x0000000000e00087
	.quad	0x0000000001000087
	.quad	0x0000000001200087
	.quad	0x0000000001400087
	.quad	0x0000000001600087
	.quad	0x0000000001800087
	.quad	0x0000000001a00087
	.quad	0x0000000001c00087
	.quad	0x0000000001e00087
	.fill	496,8,0

.align 8
.global	gdt_table
gdt_table:
	.quad	0x0000000000000000
	.quad	0x0020980000000000	/* 64bit kernel code segment, 0x08 */
	.quad	0x0000920000000000	/* 64bit kernel data segment, 0x10 */
	.quad	0x0020f80000000000	/* 64bit user code segment, 0x18 */
	.quad	0x0000f20000000000	/* 64bit user data segment, 0x20 */
	.fill	12, 8, 0		/* reserved */
.global gdt_end
gdt_end:

	.section .data
.align 8
.global	idt_table
idt_table:
	.rept 256
	.quad 0,0
	.endr
.global idt_end
idt_end:
