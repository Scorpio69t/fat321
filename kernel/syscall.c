#include <alphaz/syscall.h>
#include <alphaz/sched.h>
#include <alphaz/tty.h>
#include <asm/unistd.h>

syscall syscall_table[SYS_CALL_SIZE];


void sys_get_ticks(void)
{
    struct syscall_args_struct args;
    struct task_struct * task = current();
    struct thread_struct * thread = get_thread_info(task);
    get_syscall_args(&args, thread);

    args.arg0 = ticks;

    set_syscall_args(&args, thread);
}


void sys_write(void)
{
    int fd;
    void *buf;
    size_t n;
    struct syscall_args_struct args;
    struct task_struct * task = current();
    struct thread_struct * thread = get_thread_info(task);

    get_syscall_args(&args, thread);
    fd = (int)args.arg1;
    buf = (void *)args.arg2;
    n = (size_t)args.arg3;

    /* TODO: 这里应该根据进程的打开的文件信息进行判断 */
    if(fd == STDOUT_FILENO) {
        n = tty_write(buf, n, 0x0f);
    }

    args.arg0 = n;
    set_syscall_args(&args, thread);
}


/**
 * 初始化系统调用表
 */
static inline void setup_syscall_table(void)
{
    syscall_table[__NR_getticks] = sys_get_ticks;
    syscall_table[__NR_write] = sys_write;
}


void syscall_init(void)
{
    setup_syscall_table();
}
