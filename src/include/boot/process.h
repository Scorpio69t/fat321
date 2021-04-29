#ifndef _BOOT_PROCESS_H_
#define _BOOT_PROCESS_H_

#include <boot/cpu.h>
#include <kernel/sched.h>

typedef struct proc_struct proc_t;

int copy_context(proc_t *p, frame_t *regs);
int setup_proc_context(proc_t *proc, uint64 entry, uint64 stack_bottom);

struct proc_struct *__current(void);
struct proc_struct *__switch_to(struct proc_struct *, struct proc_struct *);

#endif
