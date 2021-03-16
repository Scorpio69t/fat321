/*
 * 内核初始化入口
 */

#include <boot/bug.h>
#include <boot/cpu.h>
#include <boot/disk.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/system.h>
#include <feng/bugs.h>
#include <feng/console.h>
#include <feng/fat32.h>
#include <feng/fcntl.h>
#include <feng/fork.h>
#include <feng/fs.h>
#include <feng/gfp.h>
#include <feng/kernel.h>
#include <feng/keyboard.h>
#include <feng/mm.h>
#include <feng/sched.h>
#include <feng/slab.h>
#include <feng/stdio.h>
#include <feng/unistd.h>

// static void test(void)
//{
//    char buf[10];
//    int i;
//    int fd = open("/abc/b.txt", O_RDONLY);
//    if (fd != -1) {
//        while(read(fd, buf, 3)) {
//            for (i = 0; i < 3; i++)
//                printf("%c", buf[i]);
//        }
//    } else {
//        printf("error\n");
//    }
//    printf("end\n");
//    if (close(fd))
//        printf("close error\n");
//}

// int init(void)
//{
//
//    disk_init();
//    fat32_init();
//
//    move_to_user_mode();
//
//    if (!fork()) {
//        tty_task();
//    }
//
//    while (1) {
//        sleep(1);
//    }
//
//    return 0;
//}

void kernel_main()
{
    cpu_init();
    irq_init();

    mm_init();
    kmem_cache_init();
    kmalloc_cache_init();

    keyboard_init();
    console_init();
    task_init();

    clear_screen();

    sti();
    //    kernel_thread(init, NULL, 0);
    while (1) {
        hlt();
    }
}
