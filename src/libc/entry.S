
#include <sys/syscall.h>

.global _start
_start:
	push 	%rax
	call	__libc_init
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
	movq	$NR_SENDRECV, %rax
	int	$0x80
	retq

.global sys_debug
sys_debug:
	movq	$NR_DEBUG, %rax
	int	$0x80
	retq

.data
.global _brk
_brk: .quad 0x700000000000
