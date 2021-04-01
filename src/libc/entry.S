
#include <syscall.h>

.global _start
_start:
	movq	$1, %rdi
	call 	main

.global debug
debug:
	movq	$NR_DEBUG, %rax
	int	$0x80
	retq
