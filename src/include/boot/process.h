#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <kernel/sched.h>

int copy_context(proc_t *p, frame_t *regs);
int setup_proc_context(proc_t *proc, uint64 entry, uint64 stack_bottom);

#endif
