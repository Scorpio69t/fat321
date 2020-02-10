/**
 * 进程调度相关
 */

#include <alphaz/sched.h>
#include <alphaz/list.h>
#include <alphaz/malloc.h>
#include <alphaz/string.h>
#include <alphaz/kernel.h>
#include <alphaz/unistd.h>
#include <alphaz/linkage.h>
#include <alphaz/tty.h>

#include <asm/system.h>
#include <asm/sched.h>
#include <asm/irq.h>
#include <asm/cpu.h>


/**
 * 任务链表的头结点
 */
struct list_head task_head;


/**
 * idle进程的task_struct地址
 */
struct task_struct *idle;


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

    list_for_each_entry(p, &task_head, task) {
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
struct pt_regs * get_pt_regs(struct task_struct *task)
{
    struct pt_regs *regs;
    regs = (struct pt_regs *)kernel_stack_top(task);
    regs = regs - 1;
    return regs;
}


static struct task_struct * __sched
context_switch(struct task_struct *prev, struct task_struct *next)
{
    switch_to(prev, next, prev);
    return prev;
}


/**
 * schedule是进程的调度器，该方法在就绪进程队列中选出一个进程进行切换
 * 当前进程的调度并不涉及优先级和运行时间的一系列复杂因素，仅仅是将时间片消耗完的进程
 * 移到队尾，然后选出进程状态为TASK_RUNNING的进程作为下一个的进程
 */
void __sched schedule(void)
{
    struct task_struct *prev, *next, *p;

    prev = current;
    next = NULL;

    list_del(&prev->task);
    list_add_tail(&prev->task, &task_head);
    list_for_each_entry(p, &task_head, task) {
        if (p->state == TASK_RUNNING) {
            next = p;
            break;
        }
    }
    if (!next) next = idle;
    next->counter = 1;
    prev = context_switch(prev, next);
}


/**
 * 时钟中断处理函数
 */
void do_timer(struct pt_regs *reg, unsigned nr)
{
    ticks_plus();
    update_alarm();
    schedule();
}


/**
 * sys_sleep - 进程睡眠
 * 该进程实现了秒级睡眠和毫秒级睡眠的中断处理，对应sleep和msleep两个系统调用的用户态接口，根
 * 据第一个参数的类型，来确定使用哪种类型的睡眠, 时间精度位10ms
 */
asmlinkage long __sched sys_sleep(unsigned long type, unsigned long t)
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

asmlinkage int sys_exit(int status)
{
    return status;
}

/**
 * 设置idle进程，其中栈在head.S中创建
 */
static void setup_idle_process(void)
{
	/* 其中包括内核栈 */
	struct task_struct *ts = current;

	ts->state = TASK_RUNNING;
	ts->flags |= PF_KERNEL;

	ts->stack = NULL;           /* 无用户栈 */
	ts->pid = 0;
	ts->prio = LOWEST_PRIO;
	ts->counter = 1;
	ts->alarm = 0;

	strcpy(ts->comm, "idle");

	ts->parent = NULL;
	list_head_init(&ts->children);

	ts->thread.esp0 = NULL_STACK_MAGIC;
	ts->thread.esp = NULL_STACK_MAGIC;
	ts->mm = NULL;
	ts->signal = 0;

	idle = ts;

	tss.esp0 = ts->thread.esp0;

	list_add(&ts->task, &task_head);
}

/**
 * 任务(进程)初始化，创建第一个进程
 * 该函数应该在内核全部初始化完成后调用，即放在kernel_main()最后调用，该函数会创建一个init
 * 进程，用于以后的初始化
 */
void task_init(void)
{
    list_head_init(&task_head);
    setup_counter();
    setup_idle_process();
    register_irq(0x20, do_timer);
}
