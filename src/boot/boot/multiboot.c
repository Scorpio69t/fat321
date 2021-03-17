#include <boot/boot.h>
#include <boot/multiboot.h>
#include <feng/bugs.h>
#include <feng/kernel.h>
#include <feng/string.h>
#include <feng/types.h>

struct meminfo_struct meminfo[MEMINFO_SIZE];

int check_meminfo_end(struct meminfo_struct *info)
{
    if (info->address == 0 && info->limit == 0 && info->type == 0)
        return 1;
    return 0;
}

int check_memarea_available(struct meminfo_struct *info)
{
    if (info->type == 0x1)
        return 1;
    return 0;
}

void boot_init(void)
{
    uint64 addr = to_vir((uint64)boot_info);
    if (addr & 7) {
        panic("Unaligned mbi: 0x%x\n", addr);
        return;
    }
    memset(meminfo, 0x00, sizeof(meminfo));
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            multiboot_memory_map_t *mmap;
            int                     i = 0;
            for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                 (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                 mmap =
                     (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size)) {
                meminfo[i].address = mmap->addr;
                meminfo[i].limit = mmap->len;
                meminfo[i].type = mmap->type;
                i++;
            }
        }
    }
}
