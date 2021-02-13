#ifndef _ASM_SYSTEM_H_
#define _ASM_SYSTEM_H_

#include <asm/cpu.h>
#include <alphaz/sched.h>

/* 只有内核级进程才可调用此功能, 并且调用后，进程便丢失了原来的栈信息，不能再进行函数返回 */
#define move_to_user_mode() do {                              \
    current->flags &= ~PF_KTHREAD;                             \
    unsigned long _esp = (unsigned long)user_stack_top(current);        \
    asm volatile(                                                       \
        "pushl %%ebx\n\t"                                               \
        "pushl %%eax\n\t"                                               \
        "pushfl\n\t"                                                    \
        "popl %%eax\n\t"                                                \
        "or $0x200, %%eax\n\t"      /* 开启中断 */                       \
        "pushl %%eax\n\t"                                               \
        "pushl %%edx\n\t"                                               \
        "pushl $1f\n\t"                                                 \
        "iretl\n\t"                                                     \
        "1:\t"                                                          \
        "mov %%bx, %%ax\n\t"                                            \
        "mov %%ax, %%ds\n\t"                                            \
        "mov %%ax, %%es\n\t"                                            \
        "mov %%ax, %%fs\n\t"                                            \
        :                                                               \
        :"a"(_esp), "b"(USER_DATA_SELECTOR), "d"(USER_CODE_SELECTOR));   \
} while (0)


/**
 * 切换堆栈，并调用_switch_to切换cpu硬件上下文
 * 这里进程的切换会破坏寄存器，为了确保寄存器的值是可控的，我们在这里显式的破坏所有的寄存器
 * 由于我们的程序是使用%ebp来访问局部变量的，所以在进程切换前需要保存%ebp，确保切换回来时
 * 局部变量访问正确，而压入标志基础器的值也是为了下次调度后标志寄存器的值正确。在跳转到
 * __switch_to执行前，压入了标号1处的值，这会使prev进程在下次被调度并从__switch_to中
 * ret后继续从标号1处执行
 */
#define switch_to(prev, next, last) do {                                \
    unsigned long ebx, ecx, edx, esi, edi;                              \
    asm volatile(                                                       \
        "pushfl\n\t"                                                    \
        "pushl %%ebp\n\t"                                               \
        "movl %%esp, %0\n\t"        /* 保存prev进程的栈顶 */              \
        "movl %8, %%esp\n\t"        /* 切换栈 */                         \
        "movl $1f, %1\n\t"          /* 保存prev的eip */                  \
        "pushl %9\n\t"              /* next的eip入栈 */                  \
        "jmp __switch_to\n\t"                                           \
        "1:\t"                                                          \
        "popl %%ebp\n\t"                                                \
        "popfl\n\t"                                                     \
        :"=m"(prev->thread.esp), "=m"(prev->thread.eip),                \
        "=a"(last), "=b"(ebx), "=c"(ecx), "=d"(edx), "=S"(esi), "=D"(edi) \
        :"m"(next->thread.esp), "m"(next->thread.eip),                  \
        "a"(prev), "d"(next));                                          \
} while (0)


#endif
