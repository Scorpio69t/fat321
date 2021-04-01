#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <kernel/sched.h>

int copy_context(struct proc_struct *p, frame_t *regs, int flags);
int setup_module_context(proc_t *proc, uint64 entry);

#endif
