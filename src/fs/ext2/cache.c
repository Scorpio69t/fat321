
#include "cache.h"

#include <assert.h>
#include <malloc.h>
#include <sys/list.h>

#include "ext2.h"

static struct list_head __inode_hash_map[__INDOE_HASH_MAP_SIZE];

void put_inode_map(unsigned int ino, struct inode *inode)
{
    inode_hash_map_t *node;

    node = (inode_hash_map_t *)malloc(sizeof(inode_hash_map_t));
    assert(node != NULL);
    node->inode = inode;
    node->ino = ino;
    list_add(&node->list, &__inode_hash_map[ino % __INDOE_HASH_MAP_SIZE]);
}

struct inode *get_inode_map(unsigned int ino)
{
    struct inode *    inode;
    inode_hash_map_t *pos;

    inode = NULL;
    list_for_each_entry(pos, &__inode_hash_map[ino % __INDOE_HASH_MAP_SIZE], list)
    {
        if (pos->ino == ino) {
            inode = pos->inode;
            break;
        }
    }
    return inode;
}

void init_inode_map(void)
{
    int i;

    for (i = 0; i < __INDOE_HASH_MAP_SIZE; i++) list_head_init(&__inode_hash_map[i]);
}

static struct {
    struct list_head head;
    unsigned int     block_size;
} buffer_pool;

void init_buffer_pool(unsigned int block_size)
{
    buffer_pool.block_size = block_size;
    list_head_init(&buffer_pool.head);
}

block_buffer_t *get_block_buffer(void)
{
    block_buffer_t *block_buffer;
    if (list_is_null(&buffer_pool.head)) {
        block_buffer = (block_buffer_t *)malloc(sizeof(block_buffer_t));
        assert(block_buffer != NULL);
        block_buffer->buffer = malloc(buffer_pool.block_size);
        assert(block_buffer != NULL);
        list_add(&block_buffer->list, &buffer_pool.head);
    }

    block_buffer = list_first_entry(&buffer_pool.head, block_buffer_t, list);
    list_del(&block_buffer->list);
    return block_buffer;
}

void put_block_buffer(block_buffer_t *block_buffer)
{
    assert(block_buffer != NULL);
    list_add(&block_buffer->list, &buffer_pool.head);
}
