#include <alphaz/linkage.h>
#include <alphaz/mm.h>

.global boot_info_addr

.section .bss
.align 4096
idle_stack:
    .fill 4096,1,0
stack_top:

.section ".head.text"

ENTRY(_start)

	movl 	%ebx, (boot_info_addr - __KERNEL_OFFSET)

    	# 初始化页表
	movl	$(_PTE - __KERNEL_OFFSET), %edi		# 加载页目录的地址
    	movl    $2048, %ecx         			# 初始化2048个页表项
	movl    $7, %eax            			# 页表项的低12位的属性

initpages:
	movl    %eax, (%edi)
	addl    $4096, %eax
	addl    $4, %edi
	loop    initpages

	# 切换页表
	movl	$(_PDE - __KERNEL_OFFSET), %eax
	movl    %eax, %cr3
	movl	%cr0, %eax
	orl 	$0x80000000, %eax
	movl	%eax, %cr0
	movl 	$(flushtlb), %eax
	jmp	*%eax
flushtlb:
	movl	$stack_top, %esp
    	movl	$stack_top, %ebp
	jmp     kernel_main				# 跳转到内核

boot_info_addr:
.long 0

.align 4096
_PDE:
	.long   _PTE + 0x0007 - __KERNEL_OFFSET
	.long   _PTE + 0x1007 - __KERNEL_OFFSET
	.fill   766,4,0             			# 填充中间的766的页目录
	.long   _PTE + 0x0007 - __KERNEL_OFFSET
	.long   _PTE + 0x1007 - __KERNEL_OFFSET
	.fill	254,4,0

.align 4096
_PTE:
	.fill   2048,4,0            			# 开辟2048个页表空间
