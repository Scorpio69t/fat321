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
int        _kmap(void **, void **, void **);

ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, void *buf, size_t nbytes);
int     brk(void *addr);
void *  sbrk(long size);

int register_irq(int);
int unregister_irq(int);

int bdev_read(unsigned long pos, void *buf, size_t size);
int bdev_write(unsigned long pos, void *buf, size_t size);

#endif

#endif
