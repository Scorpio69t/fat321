/*
 * 内核初始化入口
 */

#include <boot/boot.h>
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
#include <feng/syscalls.h>

static void test(void)
{
    printk("in test\n");
   char buf[10];
   int i;
   int fd = sys_open("/home/a.txt", O_RDONLY);
   if (fd != -1) {
       while(sys_read(fd, buf, 3)) {
           for (i = 0; i < 3; i++)
               printk("%c", buf[i]);
       }
   } else {
       printk("error\n");
   }
   printk("end\n");
   if (sys_close(fd))
       printk("close error\n");
}

int init(void)
{
    printk("init process\n");
    disk_init();
    printk("disk init successfully\n");
    fat32_init();
    printk("fat32 init successfully\n");

    test();
    //    if (!fork()) {
    //        tty_task();
    //    }

    cpu_idle();

    return 0;
}

void kernel_main()
{
    boot_init();
    cpu_init();
    irq_init();
    mm_init();
    // keyboard_init();
    // console_init();
    task_init();

    // clear_screen();
    printk("kernel_main\n");

    enable_interrupt();
    kernel_process(init, NULL, CLONE_VM);

    while (1) {
        hlt();
    }
}
