/*
 * 伙伴系统
 */

#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/string.h>

static struct buddy_struct buddy;

static uint32 check_order(struct page *page)
{
    uint32 offset;
    offset = page - mem_map;

    for (int i = MAX_ORDER - 1; i >= 0; i--) {
        if ((offset & ((1 << i) - 1)) == 0)
            return i;
    }
    return 0;
}

/* Must lock buddy.lock before call the function */
static inline void block_insert(struct page *page, unsigned int order)
{
    struct page *p;
    int          i;

    for (i = order; i == order && i < MAX_ORDER; i++) {
        list_for_each_entry(p, &buddy.block[i], list)
        {
            if (i < MAX_ORDER - 1) {
                if ((p + (1 << order)) == page && check_order(p) >= order + 1) {
                    list_del(&p->list);
                    page = p;
                    order++;
                    break;
                } else if ((page + (1 << order)) == p && check_order(page) >= order + 1) {
                    list_del(&p->list);
                    order++;
                    break;
                }
            }
            if (p < page)
                continue;
            break;
        }
    }
    __list_add(&page->list, p->list.prev, &p->list);
}

struct page *__alloc_pages(unsigned int order)
{
    struct page *page = NULL;

    if (order >= MAX_ORDER)
        return NULL;

    spin_lock(&buddy.lock);

    if (list_is_null(&buddy.block[order])) {
        unsigned int next;
        for (next = order + 1; next < MAX_ORDER; next++) {
            if (list_is_null(&buddy.block[next]))
                continue;
            page = list_first_entry(&buddy.block[next], struct page, list);
            list_del(&page->list);
            break;
        }
        if (next >= MAX_ORDER)
            goto fail;
        while (next > order) {
            next = next - 1;
            block_insert(page + (1 << next), next);
        }
    } else {
        page = list_first_entry(&buddy.block[order], struct page, list);
        list_del(&page->list);
    }
    list_add(&page->list, &buddy.activate);
    spin_unlock(&buddy.lock);
    return page;
fail:
    spin_unlock(&buddy.lock);
    return NULL;
}

struct page *alloc_pages(unsigned int gfp_mask, unsigned int order)
{
    struct page *ret = NULL;

    switch (gfp_mask) {
    case GFP_USER:
    case GFP_KERNEL:
        ret = __alloc_pages(order);
        if (ret) {
            ret->flags |= PF_BUSY;
        }
        break;
    default:
        ret = __alloc_pages(order);
        if (ret) {
            ret->flags |= PF_BUSY;
        }
    }
    return ret;
}

struct page *alloc_page(unsigned int gfp_mask)
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
    memset((void *)ret, 0x00, PAGE_SIZE);
    return ret;
}

void __free_pages(struct page *page, unsigned int order)
{
    spin_lock(&buddy.lock);
    block_insert(page, order);
    spin_unlock(&buddy.lock);
}

void free_pages(unsigned long addr, unsigned int order)
{
    struct page *page = NULL, *pos;
    addr = PAGE_LOWER_ALIGN(addr);
    spin_lock(&buddy.lock);
    list_for_each_entry(pos, &buddy.activate, list)
    {
        if (pos->virtual == (void *)addr) {
            list_del(&pos->list);
            page = pos;
            break;
        }
    }
    spin_unlock(&buddy.lock);
    if (!page)
        return;
    __free_pages(page, order);
}

void free_page(unsigned long addr)
{
    free_pages(addr, 0);
}

void buddy_system_init(uint32 nr_pages)
{
    struct page *left = mem_map, *right = mem_map, *tail = mem_map + nr_pages;

    spin_init(&buddy.lock);
    list_head_init(&buddy.activate);
    for (int i = 0; i < MAX_ORDER; i++) {
        list_head_init(&buddy.block[i]);
    }

    while (right != tail) {
        if (right->flags & PF_RESERVE) {
            left = ++right;
            continue;
        }
        int32 count = 0, order = check_order(left); /* Do not use unsigned type */
        while (!(right->flags & PF_RESERVE) && right != tail && count < (1 << order)) {
            count++;
            right++;
        }
        if (count == 1 << order) {
            list_add_tail(&left->list, &buddy.block[order]);
        } else {
            for (; order >= 0; order--) {
                if (count & (1 << order)) {
                    list_add_tail(&left->list, &buddy.block[order]);
                    left += 1 << order;
                }
            }
        }
        left = right;
    }

    /* 一些测试代码 */
    // for (int i = 0; i < MAX_ORDER; i++) {
    //     struct page *pos;
    //     int count = 0;
    //     list_for_each_entry(pos, &buddy.block[i], list) {
    //         count++;
    //     }
    //     printk("order %d: count %d\n", i, count);
    // }

    // struct page *p;
    // p = alloc_pages(GFP_KERNEL, 0);
    // printk("order 0: %p", p->virtual);
    // __free_pages(p, 0);
    // p = alloc_pages(GFP_KERNEL, 1);
    // printk("order 1: %p", p->virtual);
    // __free_pages(p, 1);
    // p = alloc_pages(GFP_KERNEL, 2);
    // printk("order 2: %p", p->virtual);
    // __free_pages(p, 2);
    // p = alloc_pages(GFP_KERNEL, 7);
    // printk("order 7: %p", p->virtual);
    // __free_pages(p, 7);
    // printk("=========================\n");

    // struct page *pages[1024];
    // for (int i = 0; i < 1024; i++) {
    //     pages[i] = alloc_pages(GFP_KERNEL, 0);
    // }

    // for (int i = 0; i < 1024; i++) {
    //     __free_pages(pages[i], 0);
    // }

    // for (int i = 0; i < MAX_ORDER; i++) {
    //     struct page *pos;
    //     int count = 0;
    //     list_for_each_entry(pos, &buddy.block[i], list) {
    //         count++;
    //     }
    //     printk("order %d: count %d\n", i, count);
    // }
}
