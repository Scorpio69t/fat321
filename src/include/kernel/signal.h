#ifndef _KERNEL_SIGNAL_H_
#define _KERNEL_SIGNAL_H_

#include <kernel/sched.h>

int send_signal(proc_t *proc, int signal);

#define SIGCHLD 17

#endif
