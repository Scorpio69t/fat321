#ifndef _ALPHAZ_SYSCALLS_H_
#define _ALPHAZ_SYSCALLS_H_

#include <alphaz/linkage.h>
#include <alphaz/type.h>

asmlinkage unsigned long sys_get_ticks(void);

asmlinkage pid_t sys_fork(void);
asmlinkage ssize_t sys_read(int fd, void *buf, size_t nbytes);
asmlinkage ssize_t sys_write(int fd, const void *buf, size_t nbytes);
asmlinkage int sys_exit(int status);
asmlinkage int sys_pause(void);
asmlinkage int sys_chdir(const char *path);
asmlinkage int sys_getcwd(char *buf, size_t n);
asmlinkage unsigned long sys_getpid(void);
asmlinkage long sys_sleep(void);
asmlinkage int sys_getdents(int, void *, int);

asmlinkage long sys_reboot(void);
asmlinkage long sys_debug(void);

#endif
