/*
 * 内核初始化入口
 */

#include <alphaz/bugs.h>
#include <alphaz/fat32.h>
#include <alphaz/fcntl.h>
#include <alphaz/fork.h>
#include <alphaz/fs.h>
#include <alphaz/kernel.h>
#include <alphaz/keyboard.h>
#include <alphaz/mm.h>
#include <alphaz/sched.h>
#include <alphaz/stdio.h>
#include <alphaz/gfp.h>
#include <alphaz/slab.h>
#include <alphaz/console.h>
#include <alphaz/unistd.h>

#include <boot/bug.h>
#include <boot/cpu.h>
#include <boot/disk.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/system.h>

//static void test(void)
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

//int init(void)
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
