#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <feng/sched.h>

int copy_thread(struct task_struct *p, struct pt_regs *regs, int flags);

#endif
