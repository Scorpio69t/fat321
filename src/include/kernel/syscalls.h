#ifndef _KERNEL_SYSCALLS_H_
#define _KERNEL_SYSCALLS_H_

#include <boot/cpu.h>
#include <kernel/types.h>

extern long do_fork(frame_t *);
extern long do_brk(unsigned long);

#endif
