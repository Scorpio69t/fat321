#ifndef _ALPHAZ_WAIT_H_
#define _ALPHAZ_WAIT_H_

#include <alphaz/list.h>
#include <alphaz/sched.h>
#include <alphaz/spinlock.h>

struct __wait_queue_head {
    spinlock_t lock;
    struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

struct __wait_queue {
    unsigned int flags;
    struct task_struct *task;           /* 当前进程的task_struct地址 */
    struct list_head task_list;         /* wait_queue_head挂载点 */
};
typedef struct __wait_queue wait_queue_t;

void init_wait_queue_head(wait_queue_head_t *head);
void init_wait_queue(wait_queue_t *wait, struct task_struct *task);
void sleep_on(wait_queue_head_t *head);
void interruptible_sleep_on(wait_queue_head_t *head);
void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr);

static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
    list_add(&new->task_list, &head->task_list);
}

static inline void __add_wait_queue_tail(wait_queue_head_t *head,
                                                            wait_queue_t *new)
{
    list_add_tail(&new->task_list, &head->task_list);
}

static inline void __remove_wait_queue(wait_queue_head_t *head,
                                            wait_queue_t *old)
{
    list_del(&old->task_list);
}

#define wake_up(x)                      __wake_up(x, TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE, 1)
#define wake_up_nr(x, nr)               __wake_up(x, TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE, nr)
#define wake_up_all(x)                  __wake_up(x, TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE, -1)
#define wake_up_interruptible(x)        __wake_up(x, TASK_INTERRUPTIBLE, 1)
#define wake_up_interruptible_nr(x, nr) __wake_up(x, TASK_INTERRUPTIBLE, nr)
#define wake_up_interruptible_all(x)    __wake_up(x, TASK_INTERRUPTIBLE, -1)

#endif
