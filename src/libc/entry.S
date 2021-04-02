
#include <sys/syscall.h>

.global _start
_start:
	movq	$1, %rdi
	call 	main

.global sys_send
sys_send:
	movq	$NR_SEND, %rax
	int	$0x80
	retq


.global sys_recv
sys_recv:
	movq	$NR_RECV, %rax
	int	$0x80
	retq

.global sys_sendrecv
sys_sendrecv:


.global sys_debug
sys_debug:
	movq	$NR_DEBUG, %rax
	int	$0x80
	retq
