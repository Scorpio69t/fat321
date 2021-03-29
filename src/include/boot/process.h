#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <kernel/sched.h>

int copy_context(struct proc_struct *p, struct pt_regs *regs, int flags);

#endif
