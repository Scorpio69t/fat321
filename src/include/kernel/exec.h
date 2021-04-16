#ifndef _KERNEL_EXEC_H_
#define _KERNEL_EXEC_H_

int do_execve(const char *pathname, char *const argv[], char *const envp[]);

#endif
