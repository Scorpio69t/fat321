#include <boot/boot.h>
#include <boot/bug.h>
#include <boot/div64.h>
#include <boot/io.h>
#include <boot/memory.h>
#include <feng/bugs.h>
#include <feng/config.h>
#include <feng/console.h>
#include <feng/gfp.h>
#include <feng/kernel.h>
#include <feng/malloc.h>
#include <feng/mm.h>
#include <feng/page.h>
#include <feng/string.h>
#include <feng/types.h>
#include <feng/slab.h>

struct page *mem_map;

/* 计算内存总大小，包括不可用内存 */
static uint64 calc_memsize(void)
{
    uint64 total = 0;
    for (int i = 0; i < MEMINFO_SIZE; i++) {
        struct meminfo_struct *info = &meminfo[i];
        if (check_meminfo_end(info))
            break;
        printk("%llx %llx %x\n", info->address, info->limit, info->type);
        total += info->limit;
    }
    return total;
}

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
    uint64 base, limit;
    uint32 i_begin, i_end;

    /* unavailable memory */
    for (int i = 0; i < MEMINFO_SIZE; i++) {
        struct meminfo_struct *info = &meminfo[i];
        if (check_meminfo_end(info))
            break;
        if (check_memarea_available(info))
            continue;
        base = PAGE_UPPER_ALIGN(info->address);
        limit = PAGE_LOWER_ALIGN(info->address + info->limit - base);
        i_begin = base / PAGE_SIZE;
        i_end = i_begin + limit / PAGE_SIZE;
        if (i_begin >= nr_page)
            continue;
        if (i_end >= nr_page)
            i_end = nr_page - 1;
        setup_pages_flags(mem_map + i_begin, mem_map + i_end, PF_RESERVE);
    }

    /*
     * kernel and page table have been used: 0x100000 to the end of mem_map,
     * and in addition, we do not use low 1M memory
     */
    base = to_phy(KERNEL_OFFSET);
    printk("kernel start: %x\n", base);
    uint64 mem_map_end = (uint64)(mem_map + nr_page);
    i_begin = base / PAGE_SIZE;
    i_end = PAGE_UPPER_ALIGN(to_phy(mem_map_end)) / PAGE_SIZE;
    printk("kernel and page table reserved: %d %d\n", i_begin, i_end);
    setup_pages_flags(mem_map + i_begin, mem_map + i_end, PF_RESERVE);
}

void mm_init()
{
    uint64 memsize, end_addr;
    uint32 nr_pages;

    memsize = calc_memsize();
    end_addr = setup_page_table(memsize);
    end_addr = init_pages(memsize, PAGE_UPPER_ALIGN(end_addr), &nr_pages);
    printk("end_addr: %p", end_addr);
    setup_pages_reserved(nr_pages);

    buddy_system_init(nr_pages);
    kmem_cache_test();
    // kmalloc_cache_init();
}
