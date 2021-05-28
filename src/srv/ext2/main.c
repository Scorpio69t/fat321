
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

/* We only support one partition right now. */
static struct super_block *__super_block;

#define NR_INODE_BLOCK 15
static unsigned int array_ladder[NR_INODE_BLOCK];

static int ext2_bdev_read(struct super_block *sb, unsigned long pos, void *buf, size_t size)
{
    if (bdev_read(sb->partition_offset + pos, buf, size) != 0)
        return -1;
    return 0;
}

static int ext2_bdev_write(struct super_block *sb, unsigned long pos, void *buf, size_t size)
{
    if (bdev_write(sb->partition_offset + pos, buf, size) != 0)
        return -1;
    return 0;
}

static struct super_block *get_super_block()
{
    return __super_block;
}

static struct inode *getinode(struct super_block *sb, uint32 ino)
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

    group = (ino - 1) / sb->s_inodes_per_group;
    index = (ino - 1) % sb->s_inodes_per_group;
    inode_per_block = sb->block_size / sb->s_inode_size;
    itable = index / inode_per_block;
    itable_index = index % inode_per_block;
    gd = &sb->group_desc_table[group];

    inode = (struct inode *)malloc(sb->s_inode_size);
    assert(inode != NULL);

    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;
    ext2_bdev_read(sb, (gd->bg_inode_table + itable) * sb->block_size, buffer, sb->block_size);
    *inode = *(struct inode *)(buffer + sb->s_inode_size * itable_index);
    put_inode_map(ino, inode);
    put_block_buffer(block_buffer);
    return inode;
}

// 将修改后的inode值写回磁盘
static int putinode(struct super_block *sb, uint32 ino)
{
    int group, index;
    int inode_per_block;
    int itable, itable_index;
    void *buffer;
    struct group_desc *gd;
    struct inode *inode;
    block_buffer_t *block_buffer;

    if (!(inode = get_inode_map(ino)))
        return -1;

    group = (ino - 1) / sb->s_inodes_per_group;
    index = (ino - 1) % sb->s_inodes_per_group;
    inode_per_block = sb->block_size / sb->s_inode_size;
    itable = index / inode_per_block;
    itable_index = index % inode_per_block;
    gd = &sb->group_desc_table[group];

    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;
    ext2_bdev_read(sb, (gd->bg_inode_table + itable) * sb->block_size, buffer, sb->block_size);
    *(struct inode *)(buffer + sb->s_inode_size * itable_index) = *inode;
    ext2_bdev_write(sb, (gd->bg_inode_table + itable) * sb->block_size, buffer, sb->block_size);
    put_block_buffer(block_buffer);
    return 0;
}

static int ext2_mount(int part, struct dentry *dentry)
{
    struct super_block *sb;
    struct group_desc *group_desc_table;
    struct inode *inode;
    unsigned int i, base, len;
    long partition_offset;

    init_inode_map();

    sb = (struct super_block *)malloc(sizeof(struct super_block));
    assert(sb != NULL);

    if ((partition_offset = bdev_part_read(part, 1024, sb, 1024)) < 0) {
        debug("ext2_mount: bdev_part_read failed %lld\n", partition_offset);
        panic("ext2_mount: bdev_part_read failed\n");
        return -1;
    }
    if (sb->s_magic != EXT2_SUPER_MAGIC) {
        panic("ext2: magic not match\n");
        return -1;
    }
    sb->partition_offset = (uint64)partition_offset;
    sb->block_size = (1024 << sb->s_log_block_size);
    sb->nr_group = sb->s_blocks_count / sb->s_blocks_per_group;
    if (sb->s_blocks_count % sb->s_blocks_per_group)
        sb->nr_group++;

    init_buffer_pool(sb->block_size);

    group_desc_table = (struct group_desc *)malloc(sizeof(struct group_desc) * sb->nr_group);
    assert(group_desc_table != NULL);
    ext2_bdev_read(sb, (sb->s_first_data_block + 1) * sb->block_size, group_desc_table,
                   sizeof(struct group_desc) * sb->nr_group);
    sb->group_desc_table = group_desc_table;

    __super_block = sb;

    inode = getinode(sb, EXT2_ROOT_INO);
    dentry->f_ino = EXT2_ROOT_INO;
    dentry->f_mode = inode->i_mode;
    dentry->f_size = inode->i_size;

    base = sb->block_size / sizeof(uint32);
    for (i = 0, len = 1; i < NR_INODE_BLOCK; i++) {
        if (i >= 12)
            len *= base;
        array_ladder[i] = len;
    }
    return 0;
}

// 获取inode中第n个block的block号，成功返回block号，结束返回0, 失败返回-1
static long get_block_no(struct super_block *sb, struct inode *inode, unsigned int n)
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
    if (i == NR_INODE_BLOCK)
        return -1;

    if (!(block_buffer = get_block_buffer()))
        return -1;
    block_array = (uint32 *)block_buffer->buffer;
    reminder = n - 12;
    while (depth-- && block) {
        ext2_bdev_read(sb, block * sb->block_size, block_array, sb->block_size);
        index = reminder / array_ladder[12 + depth - 1];
        reminder = reminder % array_ladder[12 + depth - 1];
        block = block_array[index];
    }
    put_block_buffer(block_buffer);
    return block;
}

/**
 * 获取一个文件第n个block内的数据，data的大小最好等于block_size, 不可小于
 * Return: - 成功返回1, 读取结束返回0
 *         - 失败为-1
 */
static int getblock(struct super_block *sb, unsigned int n, struct inode *inode, void *data)
{
    unsigned long block;

    if ((block = get_block_no(sb, inode, n)) <= 0)
        return block;
    ext2_bdev_read(sb, block * sb->block_size, data, sb->block_size);
    return 1;
}

// 将inode的i_block数组中下标为n的项的值设置为no
// 目前仅支持前12项
static int setblock(struct super_block *sb, unsigned int ino, unsigned int n, unsigned int no)
{
    struct inode *inode;

    if (n >= 12)
        return -1;
    if (!(inode = getinode(sb, ino)) || inode->i_block[n])
        return -1;
    inode->i_block[n] = no;
    inode->i_blocks += sb->block_size / 512;
    return 0;
}

static long alloc_block(struct super_block *sb)
{
    int i, j, k;
    long block;
    unsigned long pos;
    uint8 *bitmap, val;
    block_buffer_t *buffer;

    if (!(buffer = get_block_buffer()))
        return -1;
    for (i = 0; i < sb->nr_group; i++) {
        pos = sb->group_desc_table[i].bg_block_bitmap * sb->block_size;
        ext2_bdev_read(sb, pos, buffer->buffer, sb->block_size);
        bitmap = (uint8 *)buffer->buffer;
        for (j = 0; j < sb->block_size; j++) {
            val = bitmap[j];
            for (k = 0; k < 8; k++) {
                if (!(val & (1 << k))) {
                    block = i * sb->block_size + j * 8 + k;
                    goto found;
                }
            }
        }
    }

found:
    bitmap[j] |= (1 << k);
    if (ext2_bdev_write(sb, pos, bitmap, sb->block_size) < 0)
        goto failed;
    put_block_buffer(buffer);
    return block;
failed:
    put_block_buffer(buffer);
    return -1;
}

// 分配一个inode, 成功返回inode号, 失败返回-1
static long alloc_inode(struct super_block *sb)
{
    int i, j, k;
    unsigned int ino;
    unsigned long pos;
    uint8 *bitmap, val;
    block_buffer_t *buffer;

    if (!(buffer = get_block_buffer()))
        return -1;
    for (i = 0; i < sb->nr_group; i++) {
        pos = sb->group_desc_table[i].bg_inode_bitmap * sb->block_size;
        if (ext2_bdev_read(sb, pos, buffer->buffer, sb->block_size) < 0)
            goto failed;
        bitmap = (uint8 *)buffer->buffer;
        for (j = 0; j < sb->block_size; j++) {
            val = bitmap[j];
            for (k = 0; k < 8; k++) {
                if (!(val & (1 << k))) {
                    ino = i * sb->block_size + j * 8 + k + 1;  // notice that inode number start from 1
                    goto founded;
                }
            }
        }
    }
founded:
    bitmap[j] |= (1 << k);
    if (ext2_bdev_write(sb, pos, bitmap, sb->block_size) < 0)
        goto failed;
    put_block_buffer(buffer);
    return ino;
failed:
    put_block_buffer(buffer);
    return -1;
}

static int ext2_lookup(const char *filename, ino_t pino, struct dentry *dentry)
{
    struct inode *pinode, *inode;
    struct super_block *sb;
    struct linked_directory *dir;
    block_buffer_t *block_buffer;

    unsigned int block, ino, filename_len;
    void *buffer;

    assert((sb = get_super_block()) != NULL);
    filename_len = strlen(filename);
    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;
    if (!(pinode = getinode(sb, pino)))
        return -1;
    block = 0;
    ino = 0;
    while (getblock(sb, block, pinode, buffer) > 0) {
        dir = (struct linked_directory *)buffer;
        int offset = 0, reclen;
        while (offset != sb->block_size) {
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
    inode = getinode(sb, ino);
    dentry->f_ino = ino;
    dentry->f_mode = inode->i_mode;
    dentry->f_size = inode->i_size;
    return 0;
}

static ssize_t ext2_read(ino_t ino, void *buf, off_t pos, size_t size)
{
    struct inode *inode;
    struct super_block *sb;
    block_buffer_t *block_buffer;
    unsigned long block, offset;
    ssize_t readsz, copysz;
    void *buffer;

    assert((sb = get_super_block()) != NULL);
    if (!(inode = getinode(sb, ino)))
        return -1;
    if (pos < 0 || pos >= inode->i_size)
        return 0;
    if (pos + size >= inode->i_size)
        size = inode->i_size - pos;

    block_buffer = get_block_buffer();
    buffer = block_buffer->buffer;

    readsz = 0;
    block = pos / sb->block_size;
    offset = pos % sb->block_size;
    while (size > 0 && getblock(sb, block, inode, buffer) > 0) {
        copysz = sb->block_size - offset;
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
    struct inode *inode;
    struct super_block *sb;
    unsigned long offset, copysz, count;
    int sblock, block, n;
    block_buffer_t *buffer;

    if (!(sb = get_super_block()))
        return -1;
    if (!(inode = getinode(sb, ino)))
        return -1;
    if (pos < 0 || pos > inode->i_size)
        return -1;
    sblock = upper_div(pos + size, 512);
    n = inode->i_blocks / (sb->block_size / 512);
    while (inode->i_blocks < sblock) {
        if ((block = alloc_block(sb)) < 0)
            return -1;
        if (setblock(sb, ino, n, block) < 0)
            return -1;
    }

    if (!(buffer = get_block_buffer()))
        return -1;
    n = pos / sb->block_size;
    offset = pos % sb->block_size;
    count = 0;
    while (size > 0) {
        if (getblock(sb, n, inode, buffer->buffer) <= 0)
            return -1;
        copysz = sb->block_size - offset < size ? sb->block_size - offset : size;
        memcpy(buffer->buffer + offset, buf, copysz);
        if ((block = get_block_no(sb, inode, n)) <= 0)
            return -1;
        if (ext2_bdev_write(sb, block * sb->block_size, buffer->buffer, sb->block_size) < 0)
            return -1;
        n++;
        offset = 0;
        count += copysz;
        size -= copysz;
        inode->i_size += copysz;
    }
    putinode(sb, ino);
    return count;
}

static int ext2_create(ino_t pino, char *name, mode_t mode)
{
    return -1;
}

static long init_newdir(struct super_block *sb, ino_t pino, mode_t mode)
{
    long ino;
    struct inode *inode, *pinode;
    unsigned int block, offset;
    struct linked_directory *dir;
    block_buffer_t *buffer;

    if ((ino = alloc_inode(sb)) < 0)
        return -1;
    if (!(inode = getinode(sb, ino)))
        return -1;
    if (!(pinode = getinode(sb, pino)))
        return -1;
    memset(inode, 0x00, sizeof(struct inode));
    inode->i_mode = mode;
    // TODO:
    inode->i_gid = pinode->i_gid;
    inode->i_uid = pinode->i_uid;

    if ((block = alloc_block(sb)) < 0)
        return -1;
    if (setblock(sb, ino, 0, block) != 0)
        return -1;

    inode->i_size = sb->block_size;
    inode->i_links_count = 1;

    if (!(buffer = get_block_buffer()))
        return -1;
    memset(buffer->buffer, 0x00, sb->block_size);
    /* create default dir */
    dir = (struct linked_directory *)buffer->buffer;
    dir->file_type = EXT2_FT_DIR;
    dir->inode = ino;
    strncpy(dir->name, ".", 1);
    dir->name_len = 1;
    dir->rec_len = (sizeof(struct linked_directory) + dir->name_len + 0x3) & -(uint16)0x4;  // align 4

    offset = dir->rec_len;
    dir = (struct linked_directory *)(buffer->buffer + offset);
    dir->file_type = EXT2_FT_DIR;
    dir->inode = pino;
    strncpy(dir->name, "..", 2);
    dir->name_len = 2;
    dir->rec_len = sb->block_size - offset;

    ext2_bdev_write(sb, block * sb->block_size, buffer->buffer, sb->block_size);
    putinode(sb, ino);
    return ino;
}

static int ext2_mkdir(ino_t pino, char *name, mode_t mode)
{
    struct super_block *sb;
    struct inode *pinode;
    block_buffer_t *buffer;
    struct linked_directory *dir, *newdir;
    unsigned int n, block, status, namelen, freelen, dirsz;
    unsigned int ino;

    if (!(sb = get_super_block()))
        return -1;
    if (!(pinode = getinode(sb, pino)))
        return -1;
    if (!(buffer = get_block_buffer()))
        return -1;
    n = 0;
    newdir = NULL;
    namelen = strlen(name);
    while ((status = getblock(sb, n, pinode, buffer->buffer)) > 0) {
        dir = (struct linked_directory *)buffer->buffer;
        int offset = 0, reclen;
        while (offset != sb->block_size) {
            if (dir->name_len == namelen && !strncmp(name, dir->name, namelen))  // 名称重复
                goto failed;
            reclen = dir->rec_len;
            dirsz = (sizeof(struct linked_directory) + dir->name_len + 0x3) & -(unsigned int)0x4;  // 4字节对齐
            freelen = reclen - dirsz;
            if (freelen >= sizeof(struct linked_directory) + namelen) {
                newdir = (struct linked_directory *)(buffer->buffer + offset + dirsz);
                newdir->rec_len = dir->rec_len - dirsz;
                dir->rec_len = dirsz;
                block = get_block_no(sb, pinode, n);
            }
            offset += reclen;
            dir = (struct linked_directory *)(buffer->buffer + offset);
        }
        n++;
    }
    if (!newdir) {
        if ((block = alloc_block(sb)) < 0)
            goto failed;
        if (setblock(sb, pino, n, block) < 0)
            goto failed;
        putinode(sb, pino);
        newdir = (struct linked_directory *)buffer->buffer;
        newdir->rec_len = sb->block_size;
    }
    newdir->file_type = EXT2_FT_DIR;
    strncpy(newdir->name, name, namelen);
    newdir->name_len = namelen;
    if ((ino = init_newdir(sb, pino, mode)) < 0)
        goto failed;
    newdir->inode = ino;
    ext2_bdev_write(sb, block * sb->block_size, buffer->buffer, sb->block_size);
    put_block_buffer(buffer);
    return 0;
failed:
    put_block_buffer(buffer);
    return -1;
}

static int ext2_stat(ino_t ino, struct stat *buf)
{
    struct inode *inode;
    struct super_block *sb;

    assert((sb = get_super_block()) != NULL);
    if (!(inode = getinode(sb, ino)))
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
    struct super_block *sb;
    unsigned long block, offset, count, dirpcount, d_reclen;
    block_buffer_t *buffer = NULL;
    struct linked_directory *dir;

    assert((sb = get_super_block()) != NULL);
    if (!(inode = getinode(sb, ino)))
        goto failed;
    if (!(buffer = get_block_buffer()))
        goto failed;
    block = pos / sb->block_size;
    offset = pos % sb->block_size;
    count = dirpcount = 0;
    // 注意reclen在linked_directory和dirent中含义不同
    while (getblock(sb, block, inode, buffer->buffer) > 0) {
        dir = (struct linked_directory *)(buffer->buffer + offset);
        while (offset != sb->block_size) {
            d_reclen = offsetof(struct dirent, d_name) + dir->name_len + 1;
            if (nbytes < d_reclen)
                goto finish;
            dirp->d_ino = dir->inode;
            dirp->d_type = dir->file_type;
            strncpy(dirp->d_name, dir->name, dir->name_len);
            dirp->d_name[dir->name_len] = 0;
            dirp->d_reclen = d_reclen;
            dirp->d_off = block * sb->block_size + offset;

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
    .fs_mount = ext2_mount,
    .fs_mkdir = ext2_mkdir,
    .fs_create = ext2_create,
};

int main(int argc, char *argv[])
{
    run_fs(&ext2_ops);
    return 0;
}
