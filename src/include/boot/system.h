#ifndef _BOOT_SYSTEM_H_
#define _BOOT_SYSTEM_H_

#include <boot/cpu.h>
#include <feng/sched.h>

/* 不保存rax, rbp需要放在最后 TODO: rdi和rsi需要保存吗 */
#define SWITCH_SAVE   \
    "pushq %%rdi\n\t" \
    "pushq %%rsi\n\t" \
    "pushq %%rdx\n\t" \
    "pushq %%rcx\n\t" \
    "pushq %%r8\n\t"  \
    "pushq %%r9\n\t"  \
    "pushq %%r10\n\t" \
    "pushq %%r11\n\t" \
    "pushq %%rbx\n\t" \
    "pushq %%r12\n\t" \
    "pushq %%r13\n\t" \
    "pushq %%r14\n\t" \
    "pushq %%r15\n\t" \
    "pushq %%rbp\n\t"

#define SWITCH_RESTORE \
    "popq %%rbp\n\t"   \
    "popq %%r15\n\t"   \
    "popq %%r14\n\t"   \
    "popq %%r13\n\t"   \
    "popq %%r12\n\t"   \
    "popq %%rbx\n\t"   \
    "popq %%r11\n\t"   \
    "popq %%r10\n\t"   \
    "popq %%r9\n\t"    \
    "popq %%r8\n\t"    \
    "popq %%rcx\n\t"   \
    "popq %%rdx\n\t"   \
    "popq %%rsi\n\t"   \
    "popq %%rdi\n\t"

#define switch_to(prev, next, last)                                                       \
    do {                                                                                  \
        struct task_struct *__last;                                                       \
        asm volatile("pushfq\n\t" SWITCH_SAVE                                             \
                     "movq %%rsp, %0\n\t"        /* save prev rsp */                      \
                     "movq %3, %%rsp\n\t"        /* restore next rsp */                   \
                     "leaq 1f(%%rip), %%rax\n\t" /* label 1 address */                    \
                     "movq %%rax, %1\n\t"                                                 \
                     "pushq %4\n\t" /* __switch_to return address */                      \
                     "jmp __switch_to\n\t"                                                \
                     "1:\t" SWITCH_RESTORE "popfq\n\t"                                    \
                     : "=m"(prev->thread.rsp), "=m"(prev->thread.rip), "=a"(__last)       \
                     : "m"(next->thread.rsp), "m"(next->thread.rip), "D"(prev), "S"(next) \
                     : "memory", "cc");                                                   \
        last = __last;                                                                    \
    } while (0)

#endif
