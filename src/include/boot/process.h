#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <feng/sched.h>

pid_t _kernel_thread(struct pt_regs *, int (*fn)(void), void *, unsigned long);

int setup_thread(struct task_struct *, struct pt_regs *, int);

#endif
