#ifndef _KERNEL_EXEC_H_
#define _KERNEL_EXEC_H_

int do_execve(const char *pathname, char *const argv[], char *const envp[]);
void *setup_args(char *const argv[], char *const envp[], void *bufend, int bufsize);

#endif
