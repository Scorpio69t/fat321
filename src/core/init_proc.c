#include <kernel/sched.h>

union proc_union init_proc_union __attribute__((__section__(".data.init_proc"))) = {
    INIT_PROC(init_proc_union.proc),
};
