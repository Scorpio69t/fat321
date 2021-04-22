#ifndef _EXT2_CACHE_H_
#define _EXT2_CACHE_H_

#include "ext2.h"
#include <sys/list.h>

typedef struct inode_hash_map_list {
    struct list_head list;
    unsigned int ino;
    struct inode *inode;
} inode_hash_map_t;

#endif
