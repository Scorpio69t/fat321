#ifndef _ASM_SCHED_H_
#define _ASM_SCHED_H_

#include <boot/cpu.h>
#include <feng/compiler.h>
#include <feng/types.h>

void setup_counter(void);

struct task_struct *__current(void);

struct task_struct *__regparm3 __switch_to(struct task_struct *, struct task_struct *);

#endif
