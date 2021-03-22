#include <feng/sched.h>

union task_union init_task_union __attribute__((__section__(".data.init_task"))) = {
    INIT_TASK(init_task_union.task),
};
