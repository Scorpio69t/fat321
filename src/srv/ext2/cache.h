#ifndef _EXT2_CACHE_H_
#define _EXT2_CACHE_H_

#include <sys/list.h>

#include "ext2.h"

#define __INDOE_HASH_MAP_SIZE 1024

typedef struct inode_hash_map_list {
    struct list_head list;
    unsigned int ino;
    struct inode *inode;
} inode_hash_map_t;

void put_inode_map(unsigned int, struct inode *);
struct inode *get_inode_map(unsigned int);
void init_inode_map(void);

typedef struct block_buffer {
    struct list_head list;
    void *buffer;
} block_buffer_t;

block_buffer_t *get_block_buffer(void);
void put_block_buffer(block_buffer_t *);
void init_buffer_pool(unsigned int);
#endif
