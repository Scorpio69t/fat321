#include <feng/sched.h>

union task_union init_task_union = {
    INIT_TASK(init_task_union.task),
};
