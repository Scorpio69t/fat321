
#include <feng/linkage.h>

.section .data

.global sys_call_table
sys_call_table:
    .quad  sys_get_ticks
    .quad  sys_debug    /* 1 */
    .quad  sys_fork
    .quad  sys_read
    .quad  sys_write
    .quad  sys_exit    /* 5 */
    .quad  sys_open
    .quad  sys_close
    .quad  sys_pause
    .quad  sys_debug
    .quad  sys_debug    /* 10 */
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug    /* 15 */
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_chdir
    .quad  sys_getcwd
    .quad  sys_getpid    /* 20 */
    .quad  sys_sleep
    .quad  sys_getdents
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug    /* 25 */
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_debug
    .quad  sys_reboot    /* 30 */
    .quad  sys_debug

