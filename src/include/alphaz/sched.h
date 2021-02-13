#ifndef _ALPHAZ_SCHED_H_
#define _ALPHAZ_SCHED_H_

#define KERNEL_STACK_SIZE  4096   /* 内核栈的大小 */
#define USER_STACK_SIZE    4096   /* 用户栈的大小 */

/* 进程状态 */
#define TASK_RUNNING		    (1 << 0)        /* 可运行状态 */
#define TASK_INTERRUPTIBLE	    (1 << 1)        /* 可中断睡眠状态 */
#define	TASK_UNINTERRUPTIBLE	(1 << 2)        /* 不可中断睡眠状态 */
#define	TASK_ZOMBIE		        (1 << 3)        /* 僵尸状态 */
#define	TASK_STOPPED		    (1 << 4)        /* 停止状态 */

/* 进程标示 */
#define PF_KTHREAD              (1 << 0)        /* 内核级进程 */
#define NEED_SCHEDULE           (1 << 1)        /* 进程需要调度标示 */

#ifndef __ASSEMBLY__

#include <alphaz/fs.h>
#include <alphaz/stdio.h>
#include <alphaz/type.h>
#include <alphaz/list.h>
#include <alphaz/string.h>

#include <asm/atomic.h>
#include <asm/sched.h>
#include <asm/cpu.h>

#define __ticks_data __attribute__((section(".data")))
#define __pid_data  __attribute__((section(".data")))
#define __sched __attribute__((section(".sched.text")))

#define current __current()

#define NULL_STACK_MAGIC    0x1234      /* 空栈魔数，便于识别错误 */

void task_init(void);
void schedule(void);
struct pt_regs * get_pt_regs(struct task_struct *task);
void copy_thread(struct thread_struct *, struct thread_struct *);


/* 进程链表头 */
extern struct list_head task_head;

#define HZ  100     /* 时钟中断频率100hz */

/**
 * 计数器的初值，它定义为ticks溢出前300s处
 * 之所以定义这样的初值是为了使因溢出造成的问题尽早暴露
 */
#define INIT_TICKS  ((unsigned long)(unsigned int)(-300*HZ))

/**
 * 时钟中断计数器，也是执行进程调度程序的次数
 */
extern unsigned long volatile __ticks_data ticks;


/* 用户进程pid的起始值 */
#define INIT_PID   ((pid_t)1024)

/**
 * pid分配的起始位置
 */
extern pid_t volatile __pid_data pid;

#define TASK_COMM_LEN      32      /* 进程名的长度 */
#define TASK_MAX_FILE      64      /* 一个进程最多打开的文件数 */
/* 进程优先级 */
#define  LOWEST_PRIO            9999            /* 进程的最低优先级 */

struct mm_struct {
    unsigned long flags;

    unsigned long *pgd;              /* 页目录所在的起始逻辑地址 */

    unsigned long start_code, end_code;
    unsigned long start_data, end_data;
    unsigned long start_brk, brk, start_stack;
};

struct files_struct {
    atomic_t   count;
    struct file *files[TASK_MAX_FILE];
};

/* 进程标示和状态标示的前面不可在定义任何变量，因为这两个变量需要在entry.S中借助偏移来访问 */
struct task_struct
{
    volatile long state;    /* 进程状态，-1不可运行，0可运行 */
    unsigned long flags;    /* 状态标识 */

    void *stack;            /* 用户栈 */
    pid_t pid;              /* 进程id */
    unsigned short prio;    /* 进程优先级 */
    unsigned long  counter; /* 进程可用时间片 */
    unsigned long  alarm;   /* 滴答数定时器 */

    char comm[TASK_COMM_LEN];           /* 进程名称 */

    struct task_struct *parent;         /* 父进程 */
    struct list_head children;          /* 子进程链表 */
    struct list_head sibling;           /* 连接到父进程的子进程链表 */

    struct thread_struct thread;        /* cpu的上下文信息 */
    struct mm_struct *mm;               /* 内存空间分布 */

    long signal;            /* 进程持有的信号 */

    struct list_head task;

    struct dentry *cwd;
    struct files_struct *files;

    /*
     * TODO: 进程所用内存空间的描述，包括页表，空间大小等
     */
};


extern struct task_struct *idle;


/**
 * 内核栈的定义方式，task_struct和其内核栈共用一片内存区域
 * task_struct使用低端内存，内核栈使用高端内存，该联合体只是想表达内核栈的定义方式
 */
union task_union
{
    struct task_struct task;
    unsigned long stack[KERNEL_STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8))) ;


/**
 * 获取当前内核栈的栈底，减8是防止i386下没有内核栈的切换时访问ss和esp寄存器引发缺页异常
 */
#define kernel_stack_top(task) (((u8 *)task) + KERNEL_STACK_SIZE - 8)

/**
 * 获取用户栈的栈底
 *
 * 这里之所以减去32的原因是因为gcc为了提高函数调用时的速度，会先在栈上sub出一部分空间，然后采
 * 用诸如movl $1, (%esp)的方式进行传参，而不是使用push，在普通的程序中这样确实不错，但是我
 * 遇到当我在kernel_main中切换了栈后，如果还使用这种方式传参，却没有在栈上sub出空间的话，这
 * 会使得这类传参操作会非法修改(%esp)处的内存，导致一些不相关变量的值被修改或者会造成缺页异常
 * 所以在这里要先在栈上留出一些空间，保证gcc采用这类传参方式不会出错
 */
#define user_stack_top(task) (((u8 *)task->stack) + USER_STACK_SIZE - 32)

#endif /*__ASSEMBLY__*/

#endif
