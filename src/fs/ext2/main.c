
#include <assert.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>
#include <sys/blkdev.h>
#include <sys/dentry.h>
#include <sys/fs.h>
#include <sys/list.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "cache.h"
#include "ext2.h"

static struct super_block *super_block;

#define NR_INODE_BLOCK 15
static unsigned int array_ladder[NR_INODE_BLOCK];

static int ext2_bdev_read(unsigned long pos, void *buf, size_t size)
{
    if (bdev_read(super_block->partition_offset + pos, buf, size) != 0)
        return -1;
    return 0;
}

static struct inode *getinode(uint32 ino)
{
    int group, index;
    int inode_per_block;
    int itable, itable_index;
    void *buffer;

    struct group_desc *gd;
    struct inode *inode;
    block_buffer_t *block_buffer;

    if ((inode = get_inode_map(ino)) != NULL)
        return inode;

    group = (ino - 1) / super_block->s_inodes_per_group;
    index = (ino - 1) % super_block->s_inodes_per_group;
    inode_per_block = super_block->block_size / super_block->s_inode_size;
    itable = index / inode_per_block;
    itable_index = index % inode_per_block;
    gd = &super_block->group_desc_table[group];

    inode = (struct inode *)malloc(super_block->s_inode_size);
    assert(inode != NULL);

    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;
    ext2_bdev_read((gd->bg_inode_table + itable) * super_block->block_size, buffer, super_block->block_size);
    *inode = *(struct inode *)(buffer + super_block->s_inode_size * itable_index);
    put_inode_map(ino, inode);
    put_block_buffer(block_buffer);
    return inode;
}

static int ext2_init(unsigned long partition_offset, struct dentry *dentry)
{
    struct group_desc *group_desc_table;
    struct inode *inode;
    unsigned int i, base, len;

    init_inode_map();

    super_block = (struct super_block *)malloc(sizeof(struct super_block));
    assert(super_block != NULL);

    super_block->partition_offset = partition_offset;
    ext2_bdev_read(1024, super_block, 1024);
    if (super_block->s_magic != EXT2_SUPER_MAGIC) {
        debug("ext2: magic not match\n");
        return -1;
    }
    super_block->block_size = (1024 << super_block->s_log_block_size);
    super_block->nr_group = super_block->s_blocks_count / super_block->s_blocks_per_group;
    if (super_block->s_blocks_count % super_block->s_blocks_per_group)
        super_block->nr_group++;

    init_buffer_pool(super_block->block_size);

    group_desc_table = (struct group_desc *)malloc(sizeof(struct group_desc) * super_block->nr_group);
    assert(group_desc_table != NULL);
    ext2_bdev_read((super_block->s_first_data_block + 1) * super_block->block_size, group_desc_table,
                   sizeof(struct group_desc) * super_block->nr_group);
    super_block->group_desc_table = group_desc_table;

    inode = getinode(EXT2_ROOT_INO);
    dentry->f_ino = EXT2_ROOT_INO;
    dentry->f_mode = inode->i_mode;
    dentry->f_size = inode->i_size;

    base = super_block->block_size / sizeof(uint32);
    for (i = 0, len = 1; i < NR_INODE_BLOCK; i++) {
        if (i >= 12)
            len *= base;
        array_ladder[i] = len;
    }
    return 0;
}

/**
 * 获取一个文件第n个block内的数据，data的大小最好等于block_size, 不可小于
 * Return: - 成功返回1, 读取结束返回0
 *         - 失败为-1
 */
static int getblock(unsigned int n, struct inode *inode, void *data)
{
    int i;
    unsigned int block, depth, sum, index, reminder;
    uint32 *block_array;
    block_buffer_t *block_buffer;

    for (i = 0, sum = 0, depth = 0; i < NR_INODE_BLOCK; i++) {
        if (i >= 12)
            depth++;
        if (n >= sum && n < sum + array_ladder[i]) {
            block = inode->i_block[i];
            break;
        }
        sum += array_ladder[i];
    }
    // block number to large
    if (i == NR_INODE_BLOCK)
        return -1;

    if (!(block_buffer = get_block_buffer()))
        return -1;
    block_array = (uint32 *)block_buffer->buffer;
    reminder = n - 12;
    while (depth-- && block) {
        ext2_bdev_read(block * super_block->block_size, block_array, super_block->block_size);
        index = reminder / array_ladder[12 + depth - 1];
        reminder = reminder % array_ladder[12 + depth - 1];
        block = block_array[index];
    }
    if (!block) {
        put_block_buffer(block_buffer);
        return 0;
    }

    ext2_bdev_read(block * super_block->block_size, data, super_block->block_size);
    put_block_buffer(block_buffer);
    return 1;
}

static int ext2_lookup(const char *filename, ino_t pino, struct dentry *dentry)
{
    struct inode *pinode, *inode;
    struct linked_directory *dir;
    block_buffer_t *block_buffer;

    unsigned int block, ino, filename_len;
    void *buffer;

    filename_len = strlen(filename);
    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;
    pinode = getinode(pino);
    block = 0;
    ino = 0;
    while (getblock(block, pinode, buffer) > 0) {
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

    return -1;

founded:
    inode = getinode(ino);
    dentry->f_ino = ino;
    dentry->f_mode = inode->i_mode;
    dentry->f_size = inode->i_size;
    return 0;
}

static ssize_t ext2_read(ino_t ino, void *buf, off_t pos, size_t size)
{
    struct inode *inode;
    block_buffer_t *block_buffer;
    unsigned long block, offset;
    ssize_t readsz, copysz;
    void *buffer;

    inode = getinode(ino);
    assert(inode != NULL);
    if (pos < 0 || pos >= inode->i_size)
        return 0;

    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;

    readsz = 0;
    block = pos / super_block->block_size;
    offset = pos % super_block->block_size;
    while (size > 0 && getblock(block, inode, buffer) > 0) {
        copysz = super_block->block_size - offset;
        if (copysz > size)
            copysz = size;
        memcpy(buf + readsz, buffer + offset, copysz);
        readsz += copysz;
        size -= copysz;
        block++;
        offset = 0;
    }

    put_block_buffer(block_buffer);
    return readsz;
}

static ssize_t ext2_write(ino_t ino, void *buf, off_t pos, size_t size)
{
    return -1;
}

static int ext2_stat(ino_t ino, struct stat *buf)
{
    struct inode *inode;

    if (!(inode = getinode(ino)))
        return -1;

    buf->st_ino = ino;
    buf->st_mode = inode->i_mode;
    buf->st_size = inode->i_size;
    buf->st_atime = inode->i_atime;
    buf->st_mtime = inode->i_mtime;
    buf->st_ctime = inode->i_mtime;
    return 0;
}

// 这里的返回值是指在文件系统上读取的字节数，而不是写入到dirp中的字节数，目的是便于设置vfs中
// 的文件偏移
static ssize_t ext2_getdents(ino_t ino, struct dirent *dirp, off_t pos, size_t nbytes)
{
    struct inode *inode;
    unsigned long block, offset, count, dirpcount, d_reclen;
    block_buffer_t *buffer = NULL;
    struct linked_directory *dir;

    if (!(inode = getinode(ino)))
        goto failed;
    if (!(buffer = get_block_buffer()))
        goto failed;
    block = pos / super_block->block_size;
    offset = pos % super_block->block_size;
    count = dirpcount = 0;
    // 注意reclen在linked_directory和dirent中含义不同
    while (getblock(block, inode, buffer->buffer) > 0) {
        dir = (struct linked_directory *)(buffer->buffer + offset);
        while (offset != super_block->block_size) {
            d_reclen = offsetof(struct dirent, d_name) + dir->name_len + 1;
            if (nbytes < d_reclen)
                goto finish;
            dirp->d_ino = dir->inode;
            dirp->d_type = dir->file_type;
            strncpy(dirp->d_name, dir->name, dir->name_len);
            dirp->d_name[dir->name_len] = 0;
            dirp->d_reclen = d_reclen;
            dirp->d_off = block * super_block->block_size + offset;

            dirpcount += d_reclen;
            nbytes -= d_reclen;
            offset += dir->rec_len;
            count += dir->rec_len;
            dir = (struct linked_directory *)((unsigned long)dir + dir->rec_len);
            dirp = (struct dirent *)((unsigned long)dirp + dirp->d_reclen);
        }
        block++;
    }
finish:
    put_block_buffer(buffer);
    return count;

failed:
    put_block_buffer(buffer);
    return -1;
}

static struct fs_ops ext2_ops = {
    .fs_lookup = ext2_lookup,
    .fs_read = ext2_read,
    .fs_write = ext2_write,
    .fs_stat = ext2_stat,
    .fs_getdents = ext2_getdents,
    .fs_init = ext2_init,
};

int main(int argc, char *argv[])
{
    run_fs(GPT_ROOT_TYPE_GUID, "/", &ext2_ops);
    return 0;
}
