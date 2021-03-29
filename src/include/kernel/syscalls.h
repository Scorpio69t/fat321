#ifndef _KERNEL_SYSCALLS_H_
#define _KERNEL_SYSCALLS_H_

#include <kernel/linkage.h>
#include <kernel/types.h>

unsigned long sys_get_ticks(void);
pid_t         sys_fork(void);
ssize_t       sys_read(int fd, void *buf, size_t nbytes);
ssize_t       sys_write(int fd, const void *buf, size_t nbytes);
int           sys_exit(int status);
int           sys_pause(void);
int           sys_chdir(const char *path);
int           sys_getcwd(char *buf, size_t n);
unsigned long sys_getpid(void);
long          sys_sleep(void);
int           sys_getdents(int, void *, int);
long          sys_reboot(void);
long          sys_debug(void);

#endif
