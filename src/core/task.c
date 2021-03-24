#include <feng/sched.h>

struct files_struct init_files = INIT_FILES;

union task_union init_task_union __attribute__((__section__(".data.init_task"))) = {
    INIT_TASK(init_task_union.task),
};
