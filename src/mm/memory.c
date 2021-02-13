#include <alphaz/kernel.h>
#include <alphaz/malloc.h>
#include <alphaz/mm.h>
#include <alphaz/mm_types.h>
#include <alphaz/mmzone.h>
#include <alphaz/string.h>
#include <alphaz/type.h>
#include <alphaz/gfp.h>
#include <alphaz/config.h>
#include <alphaz/page.h>
#include <alphaz/bugs.h>
#include <alphaz/console.h>

#include <asm/memory.h>
#include <asm/bug.h>
#include <asm/div64.h>
#include <asm/io.h>

struct zone mm_zones[MAX_NR_ZONES] = {
    [ZONE_KERNEL] = { .name = "KERNEL", },
    [ZONE_USER] = { .name = "USER", },
};

static unsigned long zone_begin_addr[MAX_NR_ZONES] = {
    ZONE_KERNEL_BEGIN, ZONE_USER_BEGIN,
};
static unsigned long zone_end_addr[MAX_NR_ZONES] = {
    ZONE_KERNEL_END, ZONE_USER_END,
};

struct page *mem_map;
const struct minfo *minfo_array = (struct minfo *)MEM_INFO_ADDR;

/* 判断当前minfo是否结束 */
static int is_minfo_end(struct minfo *info)
{
    return info->base_addr_low == MEM_INFO_END_MAGIC &&
             info->base_addr_high == MEM_INFO_END_MAGIC;
}

/* 计算内存总大小，包括内存空洞 */
static unsigned long long calc_total_mem(void)
{
    struct minfo *info = (struct minfo *)minfo_array;
    unsigned long long total = 0;

    while (!is_minfo_end(info)) {
        total +=
            info->length_low | ((unsigned long long)info->length_high << 32);
        info++;
    }
    return total;
}

/* 初始化pages数组, 返回pages数组的尾地址 */
static unsigned long init_pages(unsigned long long mem_size, unsigned long mem_map_addr)
{
    int i;
    unsigned long num, kn;
    void *addr;
    struct page *p;

    mem_map_addr = (mem_map_addr / PAGE_SIZE) * PAGE_SIZE + (mem_map_addr % PAGE_SIZE ? PAGE_SIZE : 0);
    mem_map = (struct page *)mem_map_addr;

    do_div(mem_size, PAGE_SIZE);
    num = (unsigned long)mem_size;

    kn = ZONE_KERNEL_END / PAGE_SIZE;
    addr = (void *)0x00;

    p = (struct page *)mem_map;
    for (i = 0; i < num; i++) {
        p->flags = 0;
        atomic_set(0, &p->_count);
        list_head_init(&p->list);
        if (i < kn)
            p->virtual = addr + __KERNEL_OFFSET;
        else
            p->virtual = addr - __USER_OFFSET;

        /*
         * 为什么当虚拟地址为0时要设置页属性为保留页？这来着一个很有意思的bug，当从伙伴系统
         * 分配的页的虚拟地址为0时会被误认为返回的为NULL，这意味着页分配失败，所以这里将这页
         * 保留处理
         */
        if (!p->virtual)
            p->flags |= PF_RESERVE;
        addr = addr + PAGE_SIZE;
        p++;
    }
    return (unsigned long)(mem_map + num);
}

/* 设置页的属性 */
static inline void setup_pages_flags(unsigned long bi, unsigned long ei,
                                    unsigned long flags)
{
    int i;
    struct page *p = (struct page *)mem_map;

    for (i = bi; i < ei; ++i) {
        (p + i)->flags |= flags;
    }
}

/* 根据minfo数组设置pages属性 */
static int setup_pages_from_minfo(unsigned long num)
{
    unsigned long long addr, limit;
    unsigned long type, bi, ei;
    struct minfo *info = (struct minfo *)minfo_array;

    while (!is_minfo_end(info)) {
        addr = info->base_addr_low | ((unsigned long long)info->base_addr_high << 32);
        limit = info->length_low | ((unsigned long long)info->length_high << 32);
        type = info->type;
        info++;

        if (type == 1) continue;        /* 类型1为操作系统可用的内存 */

        do_div(addr, PAGE_SIZE);
        bi = addr;                      /* 在pages数组中的起始下标 */
        if (bi >= num) continue;        /* 超过实际内存大小，大于pages数组的长度 */
        ei = bi + limit / PAGE_SIZE + (limit % PAGE_SIZE ? 1 : 0);
        setup_pages_flags(bi, ei, PF_RESERVE);
    }
    return 0;
}

/* 在mem_map中标示当前已经使用的内存, 返回已使用的页数 */
static int setup_pages_reserved(unsigned long tail_addr)
{
    int i;
    unsigned long nr;

    nr = (tail_addr - __KERNEL_OFFSET) / PAGE_SIZE + 1;

    for (i = 0; i < nr; i++)
        mem_map[i].flags |= PF_RESERVE;
    return nr;
}

static int init_zones(void)
{
    int i, j;

    for (i = 0; i < MAX_NR_ZONES; i++) {
        mm_zones[i].flags = 0;
        spin_init(&mm_zones[i].lock);
        for (j = 0; j < MAX_ORDER; j++) {
            list_head_init(&mm_zones[i].free_area[j].free_list);
            mm_zones[i].free_area[j].nr_free = 0;
        }
        list_head_init(&mm_zones[i].activate);
        mm_zones[i].first_page = NULL;
        mm_zones[i].nr_pages = 0;
    }

    return 0;
}

/* 设置mm_zones数组 */
static int setup_zones(unsigned long num)
{
    int i;
    unsigned long bi, ei;   /* 起始下标和结束下标 */

    init_zones();
    for (i = 0; i < MAX_NR_ZONES; i++) {
        bi = zone_begin_addr[i] / PAGE_SIZE;
        ei = zone_end_addr[i] / PAGE_SIZE;
        if (bi >= num) continue;  // 大于实际的内存大小
        if (ei >= num) ei = num;  // 结束下标大于实际内存
        mm_zones[i].first_page = (struct page *)mem_map + bi;
        mm_zones[i].nr_pages = ei - bi;
    }

    return 0;
}

static int setup_video_reserved(unsigned long num)
{
    int i;
    unsigned int ind, nr;

    ind = __phy(VIDEO_MAP_ADDR) / PAGE_SIZE;
    nr = VIDEO_BUF_SIZE / PAGE_SIZE + (VIDEO_BUF_SIZE % PAGE_SIZE ? 1 : 0);
    if (num < ind)
        return 0;

    for (i = ind; i < num && i < nr; i++)
        mem_map[i].flags |= PF_RESERVE;
    return 0;
}

static int map_video_buf(void)
{
    unsigned long phyaddr, *pgd, *pte;
    unsigned int ind, nr, i, j;

    phyaddr = *(int *)VIDEO_BASE_ADDR;

    if (phyaddr == 0)           // 为0?说明video.S未能获取到可用的模式号
        return 0;

    ind = VIDEO_MAP_ADDR / (PAGE_SIZE * NUM_PER_PAGE);      // 计算将要进行映射的页目录的下标
    pgd = (unsigned long *)get_pgd();                       // 页目录的位置

    nr = VIDEO_BUF_SIZE / PAGE_SIZE;                        // 缓冲区页数
    for (i = ind; ;i++) {
        if (pgd[i])
            pte = (unsigned long *)__vir(pgd[i] & (~0xfffUL));
        else {
            pte = (unsigned long *)get_zeroed_page(GFP_KERNEL);
            pgd[i] = __phy((unsigned long)pte) | PAGE_ATTR;
        }
        assert(pte != NULL);
        for (j = 0; j < NUM_PER_PAGE && nr; j++, phyaddr += PAGE_SIZE, nr--)
            pte[j] = phyaddr | PAGE_ATTR;
        if (!nr) break;
    }

    return 0;
}

static void setup_console(void)
{
    console.buf = (unsigned int *)VIDEO_MAP_ADDR;
    console.width = *(int *)VIDEO_XRESOLUTION;
    console.height = *(int *)VIDEO_YRESOLUTION;
    console.pixel_size = 4;
}

void mm_init()
{
    unsigned long long memsize;
    unsigned long pagenum;
    unsigned long tail_addr;

    memsize = calc_total_mem();
#ifdef __ARCH_I386
    if (memsize >= 0xffffffff)
        memsize = 0xffffffff;
#endif

    tail_addr = reset_page_table(memsize);
    tail_addr = init_pages(memsize, tail_addr);

    do_div(memsize, PAGE_SIZE);
    pagenum = memsize;

    setup_pages_from_minfo(pagenum);
    setup_pages_reserved(tail_addr);
    setup_video_reserved(pagenum);
    setup_zones(pagenum);
    buddy_system_init();

    map_video_buf();
    setup_console();
}
