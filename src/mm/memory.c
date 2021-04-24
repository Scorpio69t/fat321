#include <boot/boot.h>
#include <boot/io.h>
#include <boot/memory.h>
#include <kernel/bugs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/multiboot.h>
#include <kernel/page.h>
#include <kernel/slab.h>
#include <kernel/string.h>
#include <kernel/types.h>

struct page *mem_map;

/* 初始化pages数组, 返回pages数组的尾地址 */
static uint64 init_pages(uint64 mem_size, uint64 mem_map_addr, uint32 *nr_page)
{
    uint32       num;
    uint64       addr;
    struct page *page;

    mem_map = (struct page *)mem_map_addr;
    num = mem_size / PAGE_SIZE;
    addr = 0x00;
    page = (struct page *)mem_map;
    for (int i = 0; i < num; i++, page++) {
        page->flags = 0;
        atomic_set(0, &page->_count);
        list_head_init(&page->list);
        page->virtual = vir_ptr(void, addr);
        addr = addr + PAGE_SIZE;
    }
    /* Do not use the first page which address is 0, because address 0 means NULL */
    mem_map->flags |= PF_RESERVE;
    if (nr_page)
        *nr_page = num;
    return (uint64)(mem_map + num);
}

/* 设置页的属性 */
static inline void setup_pages_flags(struct page *begin, struct page *end, unsigned long flags)
{
    assert(begin <= end);

    while (begin <= end) {
        begin->flags |= flags;
        begin++;
    }
}

static void setup_pages_reserved(uint32 nr_page)
{
    uint32 i_begin, i_end;

    /* unavailable memory */
    for (int i = 0; i < kinfo.mmap_size; i++) {
        multiboot_memory_map_t *mmap = &kinfo.mmap[i];
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
            continue;
        i_begin = mmap->addr / PAGE_SIZE;
        i_end = (mmap->addr + mmap->len) / PAGE_SIZE;
        if (i_begin >= nr_page)
            continue;
        if (i_end >= nr_page)
            i_end = nr_page - 1;
        setup_pages_flags(mem_map + i_begin, mem_map + i_end, PF_RESERVE);
    }

    /* mmio reserved */
    i_begin = kinfo.mmio_start / PAGE_SIZE;
    i_end = kinfo.mmio_end / PAGE_SIZE;
    if (i_begin < nr_page) {
        if (i_end >= nr_page)
            i_end = nr_page - 1;
        setup_pages_flags(mem_map + i_begin, mem_map + i_end, PF_RESERVE);
    }

    /*
     * kernel and page table have been used: 0x100000 to the end of mem_map,
     * and in addition, we do not use low 1M memory
     */
    i_begin = 0;
    i_end = to_phy(kinfo.mem_map_end) / PAGE_SIZE;
    printk("kernel and page table reserved index: %d %d\n", i_begin, i_end);
    setup_pages_flags(mem_map + i_begin, mem_map + i_end, PF_RESERVE);
}

void mm_init()
{
    uint64 memsize;
    uint32 nr_pages;
    int    i;

    kinfo.mmio_start = 0xc0000000;
    kinfo.mmio_end = 0xffffffff;

    /* 计算内存总大小，包括不可用内存 */
    for (memsize = 0, i = 0; i < kinfo.mmap_size; i++) {
        memsize += kinfo.mmap[i].len;
    }
    printk("memory size: 0x%llx\n", memsize);

    kinfo.global_pgd_start = to_phy(kinfo.kernel_end);
    for (i = 0; i < kinfo.module_size; i++) {
        if (kinfo.module[i].mod_end > kinfo.global_pgd_start)
            kinfo.global_pgd_start = kinfo.module[i].mod_end;
    }

    kinfo.global_pgd_start = PAGE_UPPER_ALIGN(to_vir(kinfo.global_pgd_start));
    kinfo.global_pgd_end = setup_page_table(memsize);

    printk("global page table: start: 0x%016llx, end 0x%016llx\n", kinfo.global_pgd_start, kinfo.global_pgd_end);

    kinfo.mem_map_start = PAGE_UPPER_ALIGN(kinfo.global_pgd_end);
    kinfo.mem_map_end = init_pages(memsize, kinfo.mem_map_start, &nr_pages);
    printk("mem_map table: start: 0x%016llx, end: 0x%016llx\n", kinfo.mem_map_start, kinfo.mem_map_end);

    setup_pages_reserved(nr_pages);

    buddy_system_init(nr_pages);
    // kmem_cache_test();
    kmalloc_cache_init();
}
