#include <libc.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/list.h>
#include <sys/syscall.h>
#include <sys/types.h>

typedef struct {
    struct list_head list;
    void *           ptr;
    size_t           size;
} m_node;

#define EACH_BRK      0x2000
#define NODE_INC_SIZE 1024
static struct list_head m_free_node;
static struct list_head m_free, m_busy;

static m_node *alloc_node(void)
{
    m_node *node;

    if (list_is_null(&m_free_node)) {
        m_node *newnodes = sbrk(NODE_INC_SIZE * sizeof(m_node));
        if (newnodes == NULL) {
            debug("alloc_node sbrk faild\n");
            return NULL;
        }
        for (int i = 0; i < NODE_INC_SIZE; i++) {
            list_add_tail(&newnodes[i].list, &m_free_node);
        }
    }
    node = list_first_entry(&m_free_node, m_node, list);
    list_del(&node->list);
    memset(node, 0x00, sizeof(m_node));

    return node;
}

static void free_node(m_node *node)
{
    list_add(&node->list, &m_free_node);
}

void __malloc_init(void)
{
    m_node *nodes, *n;
    int     i;

    list_head_init(&m_free_node);
    list_head_init(&m_free);
    list_head_init(&m_busy);

    nodes = (m_node *)sbrk(NODE_INC_SIZE * sizeof(m_node));
    if (nodes == NULL) {
        debug("__init_malloc sbrk faild\n");
        return;
    }

    for (i = 0; i < NODE_INC_SIZE; i++) {
        list_add_tail(&nodes[i].list, &m_free_node);
    }

    n = alloc_node();
    n->size = EACH_BRK;
    n->ptr = sbrk(EACH_BRK);
    list_add(&n->list, &m_free);
}

static void merge_free(m_node *node)
{
    struct list_head *pos, *n;
    m_node *          entry;
    int               merged = 0;

    list_for_each_safe(pos, n, &m_free)
    {
        entry = list_entry(pos, m_node, list);
        if (entry->ptr + entry->size == node->ptr) {
            node->ptr = entry->ptr;
            node->size += entry->size;
            list_del(&entry->list);
            free_node(entry);
        } else if (node->ptr + node->size == entry->ptr) {
            node->size += entry->size;
            list_del(&entry->list);
            free_node(entry);
        } else if (entry->ptr > node->ptr) {
            __list_add(&node->list, pos->prev, pos);
            merged = 1;
            break;
        }
    }
    if (!merged)
        list_add_tail(&node->list, &m_free);
    int count = 0;
    list_for_each_entry(entry, &m_free, list)
    {
        count++;
    }
}

void *malloc(size_t size)
{
    m_node *pos, *node, *reminder;

    size = (size + 0x7) & ~0x7;
    node = NULL;
    list_for_each_entry(pos, &m_free, list)
    {
        if (pos->size >= size) {
            node = pos;
            break;
        }
    }

    if (node == NULL) {
        if ((node = alloc_node()) == NULL) {
            debug("malloc alloc_node failed\n");
        }
        node->size = size > EACH_BRK ? size : EACH_BRK;
        if (!(node->ptr = sbrk(node->size))) {
            debug("malloc sbrk failed\n");
            return NULL;
        }
    } else {
        list_del(&node->list);
    }

    if (node->size > size) {
        if ((reminder = alloc_node()) == NULL) {
            debug("malloc alloc_node failed\n");
            return NULL;
        }
        reminder->size = node->size - size;
        reminder->ptr = node->ptr + size;
        node->size = size;
        merge_free(reminder);
    }
    list_add_tail(&node->list, &m_busy);
    return node->ptr;
}

void free(void *ptr)
{
    m_node *pos, *node;

    if (ptr == NULL)
        return;
    node = NULL;
    list_for_each_entry(pos, &m_busy, list)
    {
        if (pos->ptr == ptr) {
            node = pos;
            break;
        }
    }
    if (node) {
        list_del(&node->list);
        merge_free(node);
    }
}
