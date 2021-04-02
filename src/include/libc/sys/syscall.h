#ifndef _SYS_SYSCALL_H_
#define _SYS_SYSCALL_H_

#define NR_SEND     0
#define NR_RECV     1
#define NR_SENDRECV 2
#define NR_DEBUG    3

#ifndef __ASSEMBLY__

#include <sys/ipc.h>

extern int sys_send(int, message *);
extern int sys_recv(int, message *);
extern int sys_sendrecv(int, message *);
extern int sys_debug(char *);

#endif

#endif
