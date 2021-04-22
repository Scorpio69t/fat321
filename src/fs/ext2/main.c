
#include <bugs.h>
#include <malloc.h>
#include <string.h>
#include <sys/fs.h>
#include <sys/syscall.h>
#include <sys/list.h>
#include "cache.h"

#include "ext2.h"

#define __INDOE_HASH_MAP_SIZE 1024

static struct super_block *super_block;
static struct list_head __inode_hash_map[__INDOE_HASH_MAP_SIZE];

static void __put_inode_map(unsigned int ino, struct inode *inode)
{
    inode_hash_map_t *node;

    node = (inode_hash_map_t *)malloc(sizeof(inode_hash_map_t));
    assert(node != NULL);
    node->inode = inode;
    node->ino = ino;
    list_add(&node->list, &__inode_hash_map[ino % __INDOE_HASH_MAP_SIZE]);
}

static struct inode *__get_inode_map(unsigned int ino)
{
    struct inode *inode;
    inode_hash_map_t *pos;

    inode = NULL;
    list_for_each_entry(pos, &__inode_hash_map[ino % __INDOE_HASH_MAP_SIZE], list) {
        if (pos->ino == ino) {
            inode = pos->inode;
            break;
        }
    }
    return inode;
}

static int bdev_read(unsigned long offset, void *buf, size_t size)
{
    unsigned char nsect;
    unsigned long sector;

    nsect = size / 512;
    sector = (super_block->partition_offset + offset) / 512;
    debug("bdev_read nsect %d sector %d\n", nsect, sector);
    int x = storage_read(nsect, sector, buf);
    return x;
}

static struct inode *getinode(uint32 ino)
{
    int                group, index;
    int                inode_per_block;
    int                itable, itable_index;
    struct group_desc *gd;
    struct inode *     inode;
    void *             buffer;

    if ((inode = __get_inode_map(ino)) != NULL)
        return inode;

    group = (ino - 1) / super_block->s_inodes_per_group;
    index = (ino - 1) % super_block->s_inodes_per_group;
    inode_per_block = super_block->block_size / super_block->s_inode_size;
    itable = index / inode_per_block;
    itable_index = index % inode_per_block;
    gd = &super_block->group_desc_table[group];
    inode = (struct inode *)malloc(super_block->s_inode_size);
    assert(inode != NULL);
    buffer = malloc(super_block->block_size);
    assert(buffer != NULL);
    bdev_read((gd->bg_inode_table + itable) * super_block->block_size, buffer, super_block->block_size);
    *inode = *(struct inode *)(buffer + sizeof(struct inode) * itable_index);
    __put_inode_map(ino, inode);
    return inode;
}

static int ext2_init(unsigned long partition_offset, struct fs_entry *entry)
{
    int i;

    struct group_desc *group_desc_table;
    struct inode *     inode;

    debug("ext2_init offset %d\n", partition_offset);

    for (i = 0; i < __INDOE_HASH_MAP_SIZE; i++)
        list_head_init(&__inode_hash_map[i]);

    super_block = (struct super_block *)malloc(sizeof(struct super_block));
    assert(super_block != NULL);

    super_block->partition_offset = partition_offset;
    bdev_read(1024, super_block, 1024);
    if (super_block->s_magic != EXT2_SUPER_MAGIC) {
        debug("ext2: magic not match\n");
        return -1;
    }
    super_block->block_size = (1024 << super_block->s_log_block_size);
    super_block->nr_group = super_block->s_blocks_count / super_block->s_blocks_per_group;
    if (super_block->s_blocks_count % super_block->s_blocks_per_group)
        super_block->nr_group++;

    // TODO: 读取大小需要改进
    unsigned int size =
        upper_div(sizeof(struct group_desc) * super_block->nr_group, super_block->block_size) * super_block->block_size;

    group_desc_table = (struct group_desc *)malloc(size);
    assert(group_desc_table != NULL);
    bdev_read((super_block->s_first_data_block + 1) * super_block->block_size, group_desc_table, size);
    super_block->group_desc_table = group_desc_table;

    inode = getinode(EXT2_ROOT_INO);
    entry->inode = EXT2_ROOT_INO;
    entry->fsize = inode->i_size;
    entry->mode = inode->i_mode;
    entry->pread = EXT2_ROOT_INO;
    entry->pwrite = EXT2_ROOT_INO;
    return 0;
}

/**
 * Return: 成功为0， 失败或读取到结束 -1
 */
static int getblock(unsigned int n, struct inode *inode, void *data)
{
    unsigned int block, block_array_len;
    uint32 *     block_array;

    block_array_len = super_block->block_size / sizeof(uint32);

    block_array = NULL;
    if (n < 12) {
        if ((block = inode->i_block[n]) == 0)
            goto failed;
    } else if (n >= 12 && n < 12 + block_array_len) {
        n -= 12;
        if (inode->i_block[n] == 0) {
            goto failed;
        }
        block_array = (uint32 *)malloc(super_block->block_size);
        assert(block_array != NULL);
        bdev_read(inode->i_block[n], block_array, super_block->block_size);
        block = block_array[n];
        free(block_array);
    } else {
        debug("ext2 getblock not support\n");
        goto failed;
    }

    bdev_read(block * super_block->block_size, data, super_block->block_size);
    return 0;

failed:
    if (block_array != NULL)
        free(block_array);
    return -1;
}

static int ext2_lookup(const char *filename, struct fs_entry *p_entry, struct fs_entry *entry)
{
    struct inode *           pinode, *inode;
    struct linked_directory *dir;
    unsigned int             block, ino, filename_len;
    void *                   buffer;

    filename_len = strlen(filename);
    buffer = malloc(super_block->block_size);
    assert(buffer != NULL);
    pinode = getinode(p_entry->inode);
    block = 0;
    ino = 0;
    while (getblock(block, pinode, buffer) == 0) {
        dir = (struct linked_directory *)buffer;
        int offset = 0, reclen;
        while (offset != super_block->block_size) {
            if (dir->name_len == filename_len && !strncmp(filename, dir->name, filename_len)) {
                ino = dir->inode;
                goto founded;
            }
            reclen = dir->rec_len;
            dir = (struct linked_directory *)(buffer + offset + reclen);
            offset += reclen;
        }
        block++;
    }

    free(buffer);
    return -1;

founded:
    inode = getinode(ino);
    entry->inode = ino;
    entry->mode = inode->i_mode;
    entry->fsize = inode->i_size;
    entry->pread = ino;
    entry->pwrite = ino;
    return 0;
}

static ssize_t ext2_read(const struct fs_entry *entry, void *buf, loff_t pos, size_t size)
{
    struct inode *inode;
    unsigned long block, offset;
    ssize_t       readsz, copysz;
    void *        buffer;

    inode = getinode(entry->inode);
    assert(inode != NULL);
    if (pos < 0 || pos >= inode->i_size)
        return 0;

    buffer = malloc(super_block->block_size);
    assert(buffer != NULL);

    readsz = 0;
    block = pos / super_block->block_size;
    offset = pos % super_block->block_size;
    while (size > 0) {
        if (getblock(block, inode, buffer) < 0)
            break;
        copysz = super_block->block_size - offset;
        if (copysz > size)
            copysz = size;
        memcpy(buf + readsz, buffer + offset, copysz);
        readsz += copysz;
        size -= copysz;
        block++;
        offset = 0;
    }

    free(buffer);
    return readsz;
}

static ssize_t ext2_write(const struct fs_entry *entry, void *buf, loff_t pos, size_t size)
{
    return -1;
}

static struct fs_ops ext2_ops = {
    .fs_lookup = ext2_lookup,
    .fs_read = ext2_read,
    .fs_write = ext2_write,
    .fs_init = ext2_init,
};

int main(int argc, char *argv[])
{
    run_fs(&ext2_ops);
    return 0;
}
