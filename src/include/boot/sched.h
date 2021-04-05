#ifndef _BOOT_SCHED_H_
#define _BOOT_SCHED_H_

#include <boot/cpu.h>
#include <kernel/types.h>

void setup_counter(void);

struct proc_struct *__current(void);

struct proc_struct *__switch_to(struct proc_struct *, struct proc_struct *);

#endif
