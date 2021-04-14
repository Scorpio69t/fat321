/*
 * 内核初始化入口
 */

#include <boot/boot.h>
#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/irq.h>
#include <boot/system.h>
#include <kernel/bugs.h>
#include <kernel/fork.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/syscalls.h>
#include <kernel/types.h>

kinfo_t kinfo;

static void check_boot_info(uint64 addr)
{
    if (addr & 7) {
        panic("Unaligned mbi: 0x%x\n", addr);
        return;
    }
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            multiboot_memory_map_t *mmap;
            for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                 (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                 mmap =
                     (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size)) {
                assert(kinfo.mmap_size < KINFO_MEMMAP_SIZE);
                kinfo.mmap[kinfo.mmap_size].addr = mmap->addr;
                kinfo.mmap[kinfo.mmap_size].len = mmap->len;
                kinfo.mmap[kinfo.mmap_size].type = mmap->type;
                printk("memory zone: addr: 0x%016llx, len: 0x%016llx, type: %d\n", kinfo.mmap[kinfo.mmap_size].addr,
                       kinfo.mmap[kinfo.mmap_size].len, kinfo.mmap[kinfo.mmap_size].type);
                kinfo.mmap_size++;
            }
        } else if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            multiboot_tag_module_t *module = (multiboot_tag_module_t *)tag;
            assert(kinfo.module_size < KINFO_MODULE_SIZE);
            assert(strlen(module->cmdline) <= MULTIBOOT_MODULE_CMD_LEN);
            kinfo.module[kinfo.module_size].mod_start = module->mod_start;
            kinfo.module[kinfo.module_size].mod_end = module->mod_end;
            strcpy(kinfo.module[kinfo.module_size].cmdline, module->cmdline);
            printk("boot module: start: 0x%08llx, end: 0x%08llx, cmd: %s\n", kinfo.module[kinfo.module_size].mod_start,
                   kinfo.module[kinfo.module_size].mod_end, kinfo.module[kinfo.module_size].cmdline);
            kinfo.module_size++;

            assert(kinfo.mmap_size < KINFO_MEMMAP_SIZE);

            kinfo.mmap[kinfo.mmap_size].addr = module->mod_start;
            kinfo.mmap[kinfo.mmap_size].len = module->mod_end - module->mod_end;
            kinfo.mmap[kinfo.mmap_size].type = MULTIBOOT_MEMORY_RESERVED;
            kinfo.mmap_size++;
        }
    }
}

void kernel_main(void *boot_info)
{
    printk("booting...\n");
    memset(&kinfo, 0x00, sizeof(kinfo_t));

    extern char _kernel_start, _kernel_end;
    kinfo.kernrl_start = (uint64)&_kernel_start;
    kinfo.kernel_end = (uint64)&_kernel_end;
    printk("kernel start: 0x%016llx, end: 0x%016llx\n", kinfo.kernrl_start, kinfo.kernel_end);

    printk("checking boot info...\n");
    check_boot_info((uint64)boot_info);

    printk("Initializing cpu...\n");
    cpu_init();

    printk("Initializing interrupt...\n");
    irq_init();

    printk("Initializing memory management...\n");
    mm_init();

    printk("Initializing proc...\n");
    proc_init();

    printk("Initializing timer...\n");
    setup_counter();

    printk("kernel_main\n");

    enable_interrupt();

    cpu_idle();
}
