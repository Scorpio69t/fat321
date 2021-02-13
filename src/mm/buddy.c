/*
 * 伙伴系统
 */

#include <alphaz/mm.h>
#include <alphaz/mm.h>
#include <alphaz/mmzone.h>
#include <alphaz/kernel.h>
#include <alphaz/gfp.h>
#include <alphaz/string.h>

static int block_append(struct zone *zone, struct page *page, unsigned int count)
{
    int i, ret = 0;

    for (i = (MAX_ORDER - 1); i >= 0; --i) {
        if (count & (1 << i)) {
            list_add_tail(&page->list, &zone->free_area[i].free_list);
            zone->free_area[i].nr_free++;
            ret++;
            page += (1 << i);
        }
    }
    return ret;
}

static void block_insert(struct free_area *free, struct page *page)
{
    struct page *p;

    /* pages都是按照地址由小到大排序的 */
    list_for_each_entry(p, &free->free_list, list) {
        if (p < page) continue;
        break;
    }
    __list_add(&page->list, p->list.prev, &p->list);
    free->nr_free++;
}

/* 首先插入大块 */
static int block_insert_big_first(struct zone *zone, struct page *page,
                          unsigned int count)
{
    int i, ret = 0;

    for (i = (MAX_ORDER - 1); i >= 0; --i) {
        if (count & (1 << i)) {
            block_insert(&zone->free_area[i], page);
            ret++;
            page += (1 << i);
        }
    }
    return ret;
}

/* 首先插入小块 */
static int block_insert_small_first(struct zone *zone, struct page *page, unsigned int count)
{
    int i, ret = 0;

    for (i = 0; i < MAX_ORDER; ++i) {
        if (count & (1 << i)) {
            block_insert(&zone->free_area[i], page);
            ret++;
            page += (1 << i);
        }
    }
    return ret;
}

/*
 * 找到每一个页块，并插入到伙伴系统的链表中, 返回找到的页块数量
 */
static int init_each_pages_block(unsigned int zone)
{
    struct page *bp, *ep, *tail;
    unsigned int count, ret;

    bp = ep = mm_zones[zone].first_page;
    tail = mm_zones[zone].first_page + mm_zones[zone].nr_pages;
    ret = count = 0;

    /* 滑动窗口 */
    while (ep != tail) {
        if ((ep->flags & PF_RESERVE) || (ep + 1) == tail ||
            count >= (1 << (MAX_ORDER - 1))) {
            if (count)
                ret += block_append(&mm_zones[zone], bp, count);
            bp = ++ep;
            count = 0;
        } else {
            count++;
            ep++;
        }
    }
    return ret;
}

static struct page * alloc_page_zone(struct zone *zone, unsigned int order)
{
    unsigned int i;
    struct page *ret = NULL;

    spin_lock(&zone->lock);

    for (i = order; i < MAX_ORDER; i++) {
        if (zone->free_area[i].nr_free != 0) {
            ret = list_first_entry(&zone->free_area[i].free_list, struct page, list);
            list_del(&ret->list);
            zone->free_area[i].nr_free--;
            break;
        }
    }

    if (!ret) {
        spin_unlock(&zone->lock);
        return NULL;
    }

    if (i > order) {
        block_insert_small_first(zone, ret + (1 << order), (1 << i) - (1 << order));
    }

    list_add(&ret->list, &zone->activate);
    spin_unlock(&zone->lock);
    return ret;
}

struct page * alloc_pages(unsigned int gfp_mask, unsigned int order)
{
    struct page *ret = NULL;

    switch (gfp_mask) {
    case GFP_USER:
        ret = alloc_page_zone(&mm_zones[ZONE_USER], order);
        if (ret) {
            ret->flags |= PF_USER;
            break;
        }
    case GFP_KERNEL:
        ret = alloc_page_zone(&mm_zones[ZONE_KERNEL], order);
        if (ret) {
            ret->flags |= PF_KERNEL;
            break;
        }
    default:
        ret = alloc_page_zone(&mm_zones[ZONE_KERNEL], order);
        if (ret)
            ret->flags |= PF_KERNEL;
    }
    return ret;
}

struct page * alloc_page(unsigned int gfp_mask)
{
    return alloc_pages(gfp_mask, 0);
}

unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order)
{
    struct page *ret = alloc_pages(gfp_mask, order);
    if (!ret)
        return 0;
    return (unsigned long)ret->virtual;
}

unsigned long __get_free_page(unsigned int gfp_mask)
{
    struct page *ret = alloc_page(gfp_mask);
    if (!ret)
        return 0;
    return (unsigned long)ret->virtual;
}

unsigned long get_zeroed_page(unsigned int gfp_mask)
{
    unsigned long ret = __get_free_page(gfp_mask);
    if (!ret)
        return 0;
    memset((void *)ret, 0, PAGE_SIZE);
    return ret;
}

static void free_pages_zone(struct zone *zone, struct page *page, unsigned int order)
{
    int i;
    struct page *p;

    spin_lock(&zone->lock);

    list_del(&page->list);

    /* 检查是否有相邻的伙伴可进行合并, 最大的order即使相邻页不进行合并 */
    for (i = order; i <= order && i < MAX_ORDER - 1; i++) {
        list_for_each_entry(p, &zone->free_area[i].free_list, list) {
            if ((p + (1 << order)) == page) {
                list_del(&p->list);
                zone->free_area[i].nr_free--;
                page = p;
                order++;
                break;
            } else if ((page + (1 << order)) == p) {
                list_del(&p->list);
                zone->free_area[i].nr_free--;
                order++;
                break;
            }
        }
    }

    block_insert(&zone->free_area[order], page);
    spin_unlock(&zone->lock);
}

void __free_pages(struct page *page, unsigned int order)
{
    switch (page->flags & (PF_KERNEL | PF_USER)) {
    case PF_KERNEL:
        free_pages_zone(&mm_zones[ZONE_KERNEL], page, order);
        break;
    case PF_USER:
        free_pages_zone(&mm_zones[ZONE_USER], page, order);
        break;
    default:
        break;
    }
}

void free_pages(unsigned long addr, unsigned int order)
{
    int i;
    struct page *p, *ans = NULL;

    for (i = 0; i < MAX_NR_ZONES; i++) {
        spin_lock(&mm_zones[i].lock);
        list_for_each_entry(p, &mm_zones[i].activate, list)
            if (p->virtual == (void *)addr) {
                ans = p;
                break;
            }
        spin_unlock(&mm_zones[i].lock);
        if (ans)
            break;
    }
    if (ans)
        __free_pages(ans, order);
}

void free_page(unsigned long addr)
{
    free_pages(addr, 0);
}

void buddy_system_init(void)
{
    int i;

    for (i = 0; i < MAX_NR_ZONES; i++)
        init_each_pages_block(i);

//     int j;
//     for (i = 1; i < 2; i++)
//         for (j = 0; j < MAX_ORDER; j++)
//         printk("%s %d: %d\n", mm_zones[i].name, j, mm_zones[i].free_area[j].nr_free);

//     struct page *p;
//     p = alloc_pages(GFP_KERNEL, 0);
//     __free_pages(p, 0);
//     p = alloc_pages(GFP_KERNEL, 1);
//     __free_pages(p, 1);
//     p = alloc_pages(GFP_KERNEL, 2);
//     __free_pages(p, 2);
// printk("=========================\n");
//     for (i = 1; i < 2; i++)
//         for (j = 0; j < MAX_ORDER; j++)
//         printk("%s %d: %d\n", mm_zones[i].name, j, mm_zones[i].free_area[j].nr_free);
}
