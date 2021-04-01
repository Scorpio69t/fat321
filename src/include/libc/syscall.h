#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define NR_SEND     0
#define NR_RECV     1
#define NR_SENDRECV 2
#define NR_DEBUG    3

#ifndef __ASSEMBLY__

extern int debug(char *);

#endif

#endif
