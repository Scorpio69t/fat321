#ifndef _FENG_FORK_H_
#define _FENG_FORK_H_

#ifndef __ASSEMBLY__
#include <boot/cpu.h>
#include <feng/linkage.h>
#include <feng/sched.h>
#endif

#define CLONE_VM      (1UL << 8)
#define CLONE_FS      (1UL << 9)
#define CLONE_PTRACE  (1UL << 10)
#define CLONE_VFORK   (1UL << 11)
#define CLONE_STOPPEN (1UL << 12)
#define CLONE_SIGHAND (1UL << 13)
#define CLOSE_THREAD  (1UL << 14)

#ifndef __ASSEMBLY__

long do_fork(int clone_flags, unsigned long stack_start, struct pt_regs *regs, unsigned long stack_size);

extern void  ret_from_fork(void);
extern pid_t kernel_process(int (*fn)(void), void *args, unsigned long flags);

pid_t sys_fork(void);

#endif /* __ASSEMBLY__ */

#endif
