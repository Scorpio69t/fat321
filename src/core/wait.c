#include <alphaz/wait.h>
#include <alphaz/list.h>
#include <alphaz/sched.h>
#include <alphaz/spinlock.h>

/**
 * init_waitqueue_head - 初始化等待队列头
 * @head: 等待队列头指针
 */
inline void init_wait_queue_head(wait_queue_head_t *head)
{
    spin_init(&head->lock);
    list_head_init(&head->task_list);
}

inline void init_wait_queue(wait_queue_t *wait, struct task_struct *task)
{
    wait->flags = 0;
    wait->task = task;
}

/**
 * sleep_on - 将当前进程添加到等待队列上，睡眠类型为不可中断
 * @head: 等待队列头指针
 */
void sleep_on(wait_queue_head_t *head)
{
    wait_queue_t wait;
    init_wait_queue(&wait, current);

    current->state = TASK_UNINTERRUPTIBLE;

    spin_lock(&head->lock);
    __add_wait_queue_tail(head, &wait);
    spin_unlock(&head->lock);

    schedule();

    spin_lock(&head->lock);
    __remove_wait_queue(head, &wait);
    spin_unlock(&head->lock);
}

/**
 * interruptible_sleep_on - 将当前进程添加到等待队列上，睡眠类型为可中断
 * @head: 等待队列头指针
 */
void interruptible_sleep_on(wait_queue_head_t *head)
{
    wait_queue_t wait;
    init_wait_queue(&wait, current);

    current->state = TASK_INTERRUPTIBLE;

    spin_lock(&head->lock);
    __add_wait_queue_tail(head, &wait);
    spin_unlock(&head->lock);

    schedule();

    spin_lock(&head->lock);
    __remove_wait_queue(head, &wait);
    spin_unlock(&head->lock);
}

/**
 * __wake_up - 唤醒等待队列上的进程
 * @q:  等待队列头指针
 * @mode: 唤醒何种进程状态的进程
 * @nr: 唤醒多少进程
 */
void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr)
{
    struct list_head *tmp;
    spin_lock(&q->lock);
    /* 使用list_for_each遍历线程安全吗？*/
    list_for_each(tmp, &q->task_list) {
        wait_queue_t *curr = list_entry(tmp, wait_queue_t, task_list);
        if ((curr->task->state & mode) && nr--) {
            curr->task->state = TASK_RUNNING;
        }
    }
    spin_unlock(&q->lock);
}
