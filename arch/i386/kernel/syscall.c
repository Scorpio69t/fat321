#include <alphaz/type.h>
#include <asm/unistd.h>
#include <asm/syscall.h>
#include <asm/cpu.h>


/**
 * 根据cpu上下文获取系统调用的参数表
 */
void get_syscall_args(struct syscall_args_struct *sas,
                        struct thread_struct *thread)
{
    sas->arg0 = thread->eax;
    sas->arg1 = thread->ebx;
    sas->arg2 = thread->ecx;
    sas->arg3 = thread->edx;
    sas->arg4 = thread->esi;
    sas->arg5 = thread->edi;
}


/**
 * 设置系统调用的返回参数
 */
void set_syscall_args(struct syscall_args_struct *sas,
                        struct thread_struct *thread)
{
    thread->eax = sas->arg0;
    thread->ebx = sas->arg1;
    thread->ecx = sas->arg2;
    thread->edx = sas->arg3;
    thread->esi = sas->arg4;
    thread->edi = sas->arg5;
}


/**
 * 获取时钟中断的总次数
 */
unsigned int get_ticks(void)
{
    u32 d0 = __NR_getticks;
    asm volatile(
        "int $0x80\n\t"
        :"=&a"(d0)
        :"0"(d0));
    return d0;
}


ssize_t write(int fd, const void *buf, size_t n)
{
    u32 d0, d1, d2, d3;
    d0 = __NR_write;
    asm volatile(
        "int $0x80\n\t"
        :"=&a"(d0), "=&b"(d1), "=&c"(d2), "=&d"(d3)
        :"0"(d0), "1"(fd), "2"((u32)buf), "3"(n));
    return d0;
}


ssize_t read(int fd, const void *buf, size_t n)
{
    u32 d0, d1, d2, d3;
    d0 = __NR_read;
    asm volatile(
        "int $0x80\n\t"
        :"=&a"(d0), "=&b"(d1), "=&c"(d2), "=&d"(d3)
        :"0"(d0), "1"(fd), "2"((u32)buf), "3"(n));
    return d0;
}