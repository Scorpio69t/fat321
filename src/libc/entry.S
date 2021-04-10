
#include <sys/syscall.h>

.global _start
_start:
	push 	%rax
	call	__libc_init
	call 	main

.global _send
_send:
	movq	$NR_SEND, %rax
	int	$0x80
	retq


.global _recv
_recv:
	movq	$NR_RECV, %rax
	int	$0x80
	retq

.global _sendrecv
_sendrecv:
	movq	$NR_SENDRECV, %rax
	int	$0x80
	retq

.global _debug
_debug:
	movq	$NR_DEBUG, %rax
	int	$0x80
	retq

.data
.global _brk
_brk: .quad 0x700000000000