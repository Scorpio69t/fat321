#ifndef _KERNEL_SCHED_H_
#define _KERNEL_SCHED_H_

#define KERNEL_STACK_ORDER 1    /* 内核栈order */
#define KERNEL_STACK_SIZE  8192 /* 内核栈的大小 (1<<KERNEL_STACK_ORDER)*PAGE_SIZE */

#define USER_STACK_END   0x7ffffffff000 /* 用户栈的用户空间地址 */
#define USER_STACK_ORDER 1
#define USER_BRK_START   0x700000000000 /* 用户brk起始地址 */

/* 进程状态 */
#define PROC_RUNNABLE  (1 << 0) /* 可运行状态 */
#define PROC_RUNNING   (1 << 1) /* 正在运行 */
#define PROC_SENDING   (1 << 2) /* 等待发送消息 */
#define PROC_RECEIVING (1 << 3) /* 等待接收消息 */
#define PROC_STOPPED   (1 << 4) /* 停止状态 */

/* 进程标示 */
#define PF_KTHREAD    (1 << 0) /* 内核级进程 */
#define NEED_SCHEDULE (1 << 1) /* 进程需要调度标示 */
#define PF_IDLE       (1 << 2) /* idle进程 */

#define PROC_STATE 0x00
#define PROC_FLAGS 0x08

#ifndef __ASSEMBLY__

#include <boot/atomic.h>
#include <boot/cpu.h>
#include <boot/process.h>
#include <kernel/ipc.h>
#include <kernel/list.h>
#include <kernel/string.h>
#include <kernel/types.h>

#define current __current()

#define NULL_STACK_MAGIC 0x1234 /* 空栈魔数，便于识别错误 */

void proc_init(void);
void schedule(void);
void cpu_idle(void);

#define HZ 100 /* 时钟中断频率100hz */

/**
 * 计数器的初值，它定义为ticks溢出前300s处
 * 之所以定义这样的初值是为了使因溢出造成的问题尽早暴露
 */
#define INIT_TICKS ((unsigned long)(unsigned int)(-300 * HZ))

/**
 * 时钟中断计数器，也是执行进程调度程序的次数
 */
extern unsigned long volatile ticks;

/* 用户进程pid的起始值 */
#define INIT_PID ((pid_t)1024)

/**
 * pid分配的起始位置
 */
extern pid_t volatile pid;

#define PROC_COMM_LEN    32 /* 进程名的长度 */
#define PROC_MM_SEG_SIZE 8

/* 进程标示和状态标示的前面不可在定义任何变量，因为这两个变量需要在entry.S中借助偏移来访问 */
typedef struct proc_struct {
    volatile long state;           /* 进程状态，-1不可运行，0可运行 */
    unsigned long flags;           /* 状态标识 */
    pid_t pid;                     /* 进程id */
    unsigned long counter;         /* 进程可用时间片 */
    unsigned long alarm;           /* 滴答数定时器 */
    long signal;                   /* 进程持有的信号 */
    int exit_status;               /* 进程退出状态 */
    char comm[PROC_COMM_LEN];      /* 进程名称 */
    struct proc_struct *parent;    /* 父进程 */
    struct context_struct context; /* 进程的上下文信息 */
    struct list_head children;     /* 子进程链表 */
    struct list_head child_list;   /* 挂载到父进程的children上 */
    struct list_head proc;         /* 进程链表 */
    struct list_head hash_map;

    volatile long wait;         /* 记录正在等待哪个进程发送/接收消息 */
    struct list_head wait_proc; /* 等待当前进程接收自身消息的进程 */
    struct list_head wait_list; /* 连接到wait_proc */
    unsigned char has_intr;     /* 是否有中断消息 */
    message msg;

    struct {
        unsigned long flags;
        unsigned long pgd; /* 页目录所在的起始逻辑地址 */
        struct {
            unsigned int flags;
            unsigned long vstart;
            unsigned long vend;
        } psegs[PROC_MM_SEG_SIZE]; /* ELF Programe Segment */
        unsigned char nr_seg;
        unsigned long start_brk, brk;
        unsigned long start_stack, end_stack;
    } mm; /* 进程用户空间描述 */
} proc_t;

// clang-format off
#define INIT_FILES                              \
{                                               \
    .count = INIT_ATOMIC_T,                     \
    .files = { NULL },                          \
}
extern struct files_struct init_files;

// clang-format off
#define INIT_PROC(tsk)                          \
{                                               \
    .state = PROC_RUNNING,                      \
    .flags = PF_KTHREAD | PF_IDLE,              \
    .pid = 0,                                   \
    .alarm = 0,                                 \
    .comm = "idle",                             \
    .parent = NULL,                             \
    .context = {},                              \
    .proc = LIST_HEAD_INIT(tsk.proc),           \
    .children = LIST_HEAD_INIT(tsk.children),  \
    .mm = {},                                   \
    .signal = 0,                                \
}

/**
 * 内核栈的定义方式，proc_struct和其内核栈共用一片内存区域
 * proc_struct使用低端内存，内核栈使用高端内存，该联合体只是想表达内核栈的定义方式
 */
union proc_union {
    struct proc_struct proc;
    unsigned long      stack[KERNEL_STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8)));

extern union proc_union init_proc_union;

extern struct list_head proc_head;
extern struct proc_struct *proc_idle[NR_CPUS];

#define PROC_HASH_MAP_SIZE 256
extern struct list_head __proc_hash_map[];
proc_t *map_proc(pid_t);
void hash_proc(proc_t *proc);
void unmap_proc(pid_t pid);

void ticks_plus(void);
void update_alarm(void);

/**
 * 获取当前内核栈的栈底
 */
#define kernel_stack_top(proc) (((u8 *)proc) + KERNEL_STACK_SIZE - 8)

#endif /*__ASSEMBLY__*/

#endif
