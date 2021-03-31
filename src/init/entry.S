
.global _start
_start:
	movq	$1, %rdi
	call 	main

.global debug
debug:
	movq	$1, %rax
	int	$0x80
	retq
