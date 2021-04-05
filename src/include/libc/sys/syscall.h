#ifndef _SYS_SYSCALL_H_
#define _SYS_SYSCALL_H_

#define NR_SEND     0
#define NR_RECV     1
#define NR_SENDRECV 2
#define NR_DEBUG    3

#ifndef __ASSEMBLY__

#include <sys/ipc.h>

extern int _send(int, message *);
extern int _recv(int, message *);
extern int _sendrecv(int, message *);
extern int _debug(char *);

int   brk(void *addr);
void *sbrk(long size);

int register_irq(int no_intr);

#endif

#endif
