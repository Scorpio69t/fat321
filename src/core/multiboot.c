#include <kernel/bugs.h>
#include <kernel/kernel.h>
#include <kernel/multiboot.h>
#include <kernel/string.h>
#include <kernel/types.h>

void scan_boot_info(uint64 addr)
{
    if (addr & 7) {
        panic("Unaligned mbi: 0x%x\n", addr);
        return;
    }
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MMAP: {
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
        } break;
        case MULTIBOOT_TAG_TYPE_MODULE: {
            multiboot_tag_module_t *module = (multiboot_tag_module_t *)tag;
            assert(kinfo.module_size < KINFO_MODULE_SIZE);
            assert(strlen(module->cmdline) <= MULTIBOOT_MODULE_CMD_LEN);
            kinfo.module[kinfo.module_size].mod_start = module->mod_start;
            kinfo.module[kinfo.module_size].mod_end = module->mod_end;
            strcpy(kinfo.module[kinfo.module_size].cmdline, module->cmdline);
            printk("boot module: start: 0x%08llx, end: 0x%08llx, cmd: %s\n", kinfo.module[kinfo.module_size].mod_start,
                   kinfo.module[kinfo.module_size].mod_end, kinfo.module[kinfo.module_size].cmdline);

            kinfo.module_mmap[kinfo.module_size].addr = module->mod_start;
            kinfo.module_mmap[kinfo.module_size].len = module->mod_end - module->mod_end;
            kinfo.module_size++;
        } break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV: {
            struct multiboot_tag_bootdev *bootdev = (struct multiboot_tag_bootdev *)tag;
            printk("boot dev: %d %d %d", bootdev->biosdev, bootdev->slice, bootdev->part);
            kinfo.bootdev = bootdev->biosdev;
            kinfo.bootpart = bootdev->slice;
            kinfo.subpart = bootdev->part;
        } break;
        default:
            break;
        }
    }
}