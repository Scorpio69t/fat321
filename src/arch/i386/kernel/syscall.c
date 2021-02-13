#include <stdarg.h>
#include <alphaz/type.h>
#include <alphaz/unistd.h>
#include <alphaz/fcntl.h>
#include <alphaz/dirent.h>
#include <asm/unistd.h>
#include <asm/cpu.h>
#include <asm/io.h>

/**
 * __syscall - 用户态系统调用的总入口
 * @no: 系统调用号
 * @n:  系统调用的参数个数
 * 注意，所有可变参数的参数类型长度必须为32位
 */
unsigned long __syscall(int no, int n, ...)
{
    unsigned long eax = no, reg[3] = {0, 0, 0};
    va_list args;
    int i;

    va_start(args, n);
    for (i = 0; i < n && i < 3; i++) {
        reg[i] = va_arg(args, unsigned long);
    }
    va_end(args);

    asm volatile(
        "int $0x80\n\t"
        :"=a"(eax)
        :"a"(eax), "b"(reg[0]), "c"(reg[1]), "d"(reg[2])
        :"memory");
    return eax;
}


/**
 * 获取时钟中断的总次数
 */
unsigned int get_ticks(void)
{
    return __syscall(__NR_getticks, 0);
}


pid_t fork(void)
{
    return __syscall(__NR_fork, 0);
}


ssize_t write(int fd, const void *buf, size_t n)
{
    return __syscall(__NR_write, 3, (unsigned long)fd,
                        (unsigned long)buf, (unsigned long)n);
}


ssize_t read(int fd, const void *buf, size_t n)
{
    return __syscall(__NR_read, 3, (unsigned long)fd,
                        (unsigned long)buf, (unsigned long)n);
}

int open(const char *path, int oflag)
{
    return __syscall(__NR_open, 2, path, oflag);
}

int close(int fd)
{
    return __syscall(__NR_close, 1, fd);
}

int pause(void)
{
    return __syscall(__NR_pause, 0);
}

int chdir(const char *path)
{
    return __syscall(__NR_chdir, 1, path);
}

int getcwd(char *buf, size_t n)
{
    return __syscall(__NR_getcwd, 2, buf, n);
}

pid_t getpid(void)
{
    return __syscall(__NR_getpid, 0);
}

int getdents(int fd, void *dirent, int count)
{
    return __syscall(__NR_getdents, 3, fd, dirent, count);
}

void sleep(unsigned long second)
{
    __syscall(__NR_sleep, 2, (unsigned long)0, second);
}

void msleep(unsigned long ms)
{
    __syscall(__NR_sleep, 2, (unsigned long)1, ms);
}

void reboot(void)
{
    __syscall(__NR_reboot, 0);
}

void debug(void)
{
    __syscall(__NR_debug, 0);
}
