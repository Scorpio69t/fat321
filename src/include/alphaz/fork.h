#ifndef _ALPHAZ_FORK_H_
#define _ALPHAZ_FORK_H_

#include <alphaz/sched.h>
#include <alphaz/linkage.h>
#include <asm/cpu.h>

#define CLONE_VM        (1UL << 8)
#define CLONE_FS        (1UL << 9)
#define CLONE_PTRACE    (1UL << 10)
#define CLONE_VFORK     (1UL << 11)
#define CLONE_STOPPEN   (1UL << 12)
#define CLONE_SIGHAND   (1UL << 13)
#define CLOSE_THREAD    (1UL << 14)

long do_fork(int clone_flags, unsigned long stack_start, struct pt_regs *regs,
			unsigned long stack_size);
extern void ret_from_fork(void);
extern void kernel_thread_ret(void);

pid_t kernel_thread(int (*fn)(void), void *, unsigned long);

asmlinkage pid_t sys_fork(void);

#endif
