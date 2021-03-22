#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/memory.h>
#include <boot/sched.h>
#include <feng/compiler.h>
#include <feng/kernel.h>
#include <feng/sched.h>
#include <feng/types.h>

/**
 * setup_counter - 设置8253计数器
 *
 * pc上计数器的输入频率为1193180HZ
 */
void setup_counter(void)
{
    unsigned long t = 1193180;
    outb(0x43, 0x36); /* 使用二进制 模式3 先读写低字节再读写高字节 计数器0 */
    outb(0x40, (u8)(t / HZ));
    outb(0x40, (u8)((t / HZ) >> 8));
}

/**
 * 获取当前进程控制块的地址
 *
 * 由于进程控制块占一个页，每个页都是4k对其的，所以将%esp低12位变为零便是当前进程的进程控制块
 * 地址
 */
inline struct task_struct *__current(void)
{
    struct task_struct *cur;
    asm volatile("andq %%rsp, %0" : "=r"(cur) : "0"(~((uint64)KERNEL_STACK_SIZE - 1)));
    return cur;
}

/**
 * __switch_to - 进程切换的cpu上下文切换
 * @prev: 当前进程的进程控制块指针 in eax
 * @next: 下一个进程的进程控制块指针 in edx
 */
#include <boot/bug.h>
struct task_struct *__regparm3 __switch_to(struct task_struct *prev, struct task_struct *next)
{
    // tss.esp0 = next->thread.esp0;
    switch_pgd((unsigned long)next->mm->pgd);
    return prev;
}
