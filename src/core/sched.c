/**
 * 进程调度相关
 */

#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/memory.h>
#include <boot/sched.h>
#include <boot/system.h>
#include <feng/bugs.h>
#include <feng/kernel.h>
#include <feng/linkage.h>
#include <feng/list.h>
#include <feng/malloc.h>
#include <feng/mm.h>
#include <feng/sched.h>
#include <feng/slab.h>
#include <feng/string.h>
#include <feng/unistd.h>

struct sched_struct scheduler;

/**
 * 时钟中断计数器
 */
unsigned long volatile __ticks_data ticks = INIT_TICKS;

pid_t volatile __pid_data pid = INIT_PID;

/**
 *
 */
static inline void ticks_plus(void)
{
    ticks++;
}

static inline void update_alarm(void)
{
    struct task_struct *p;

    list_for_each_entry(p, &scheduler.task_head, task)
    {
        if (p->alarm == 0) {
            continue;
        }
        --p->alarm;
        if (p->alarm == 0) {
            p->state = TASK_RUNNING;
        }
    }
}

/**
 * 获取当前进程的cpu上下文
 */
struct pt_regs *get_pt_regs(struct task_struct *task)
{
    struct pt_regs *regs;
    regs = (struct pt_regs *)kernel_stack_top(task);
    regs = regs - 1;
    return regs;
}

static struct task_struct *context_switch(struct task_struct *prev, struct task_struct *next)
{
    switch_to(prev, next, prev);
    return prev;
}

/**
 * schedule是进程的调度器，该方法在就绪进程队列中选出一个进程进行切换
 * 当前进程的调度并不涉及优先级和运行时间的一系列复杂因素，仅仅是将时间片消耗完的进程
 * 移到队尾，然后选出进程状态为TASK_RUNNING的进程作为下一个的进程
 */
void schedule(void)
{
    struct task_struct *prev, *next, *p;

    prev = current;
    next = NULL;

    prev->flags &= ~NEED_SCHEDULE;
    disable_interrupt();
    list_del(&prev->task);
    list_for_each_entry(p, &scheduler.task_head, task)
    {
        if (p->state == TASK_RUNNING) {
            next = p;
            break;
        }
    }
    if (!next)
        next = scheduler.idle;
    next->counter = 1;
    prev = context_switch(prev, next);
    list_add_tail(&prev->task, &scheduler.task_head);
    enable_interrupt();
}

/**
 * 时钟中断处理函数
 */
void do_timer(struct pt_regs *reg, unsigned nr)
{
    ticks_plus();
    update_alarm();
    printk("do_timer ");
    current->flags |= NEED_SCHEDULE;
}

/**
 * sys_sleep - 进程睡眠
 * 该进程实现了秒级睡眠和毫秒级睡眠的中断处理，对应sleep和msleep两个系统调用的用户态接口，根
 * 据第一个参数的类型，来确定使用哪种类型的睡眠, 时间精度位10ms
 */
long __sched sys_sleep(unsigned long type, unsigned long t)
{
    if (type == 0) {
        current->alarm = t * HZ;
    } else if (type == 1) {
        current->alarm = t / (1000 / HZ);
    } else {
        printk("sys_sleep error\n");
        return -1;
    }
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}

int sys_pause(void)
{
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}

int sys_exit(int status)
{
    return status;
}

/**
 * 任务(进程)初始化，创建第一个进程
 * 该函数应该在内核全部初始化完成后调用，即放在kernel_main()最后调用，该函数会创建一个init
 * 进程，用于以后的初始化
 */
void task_init(void)
{
    list_head_init(&scheduler.task_head);
    scheduler.idle = &init_task_union.task;
    setup_counter();
    register_irq(0x20, do_timer);
}
