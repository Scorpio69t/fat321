#ifndef _ALPHAZ_SYSCALL_H_
#define _ALPHAZ_SYSCALL_H_

#include <alphaz/type.h>

#define STDIN_FILENO        0
#define STDOUT_FILENO       1
#define STDERR_FILENO       2

#define NR_SYSCALL   32

extern unsigned int get_ticks(void);
extern pid_t fork(void);
extern ssize_t write(int fd, const void *buf, size_t n);
extern ssize_t read(int fd, const void *buf, size_t n);
extern int close(int fd);
extern int pause(void);
extern int chdir(const char *path);
extern int getcwd(char *buf, size_t n);
extern pid_t getpid(void);
extern int getdents(int fd, void *dirent, int count);
extern void sleep(unsigned long second);
extern void msleep(unsigned long ms);
extern void reboot(void);
extern void debug(void);

#endif
