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
#include <kernel/bugs.h>
#include <kernel/console.h>
#include <kernel/fat32.h>
#include <kernel/fcntl.h>
#include <kernel/fork.h>
#include <kernel/fs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/keyboard.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/stdio.h>
#include <kernel/syscalls.h>
#include <kernel/unistd.h>

// static void test(void)
// {
//     printk("in test\n");
//     char buf[10];
//     int  i;
//     int  fd = sys_open("/home/a.txt", O_RDONLY);
//     if (fd != -1) {
//         while (sys_read(fd, buf, 3)) {
//             for (i = 0; i < 3; i++) printk("%c", buf[i]);
//         }
//     } else {
//         printk("error\n");
//     }
//     printk("end\n");
//     if (sys_close(fd))
//         printk("close error\n");
// }

int init(void)
{
    printk("init process\n");
    // disk_init();
    // printk("disk init successfully\n");
    // fat32_init();
    // printk("fat32 init successfully\n");

    // test();
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
    proc_init();

    // clear_screen();
    printk("kernel_main\n");

    enable_interrupt();
    // kernel_proc(init, NULL, CLONE_VM);
    int a;
    while (1) {
        printk("%d\n", a++);
        hlt();
    }
}
