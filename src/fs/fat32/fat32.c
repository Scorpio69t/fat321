/*
 * fat32文件系统
 * 为了简单起见，不再兼容windows对目录项的创建方式，并且，对于每一个短目录项，都要有其对应的
 * 长目录项
 */

#include "fat32.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <sys/fs.h>
#include <sys/syscall.h>

static unsigned int getnextclus(superblock_t *sb, unsigned int entry)
{
    static unsigned int buf[128];
    memset(buf, 0, 512);
    storage_read(1, sb->fat1_first_sector + (entry >> 7), buf);
    return buf[entry & 0x7f] & 0x0fffffff;
}

/* 根据短目录项获取文件名, 返回文件名的长度
 * 传入begin是防止越界
 */
static int getname(char *buf, dir_t *de, void *begin)
{
    int          len, i, j;
    static short tmp[512];

    j = 0;
    ldir_t *lde = (ldir_t *)de - 1;
    while ((void *)de >= begin && lde->LDIR_Attr == ATTR_LONG_NAME && lde->LDIR_Ord != 0xe5) {
        for (i = 0; i < 5; i++) tmp[j++] = lde->LDIR_Name1[i];
        for (i = 0; i < 6; i++) tmp[j++] = lde->LDIR_Name2[i];
        for (i = 0; i < 2; i++) tmp[j++] = lde->LDIR_Name3[i];
        lde--;
    }
    for (len = 0; len < j;) {
        buf[len] = (char)tmp[len];
        len++;
        if (!buf[len - 1])
            break;
    }
    return len; /* 包含\0的长度 */
}

static int match_name(const char *name, dir_t *de, void *begin)
{
    static char tmp[512];
    int         len;

    len = getname(tmp, de, begin);
    if (len <= 0)
        return 0;
    if (strncmp(tmp, name, len))
        return 0;
    return 1;
}

// static int fat32_readdir(struct file *filp, void *dirent, filldir_t filldir)
// {
//     fat32_directory_t *p;
//     struct fat32_private_info *private;
//     unsigned long  sector, clus, index, offset;
//     loff_t         pos, size;
//     unsigned char *buffer;
//     static char    name[512];
//     int            len, i, type;
//     u64            time;

//    private
//     = filp->f_dentry->d_sb->s_fs_info;
//     assert(private != NULL);
//     pos = filp->f_pos;
//     // panic("%d\n", filp->f_dentry->d_inode->i_size);
//     // if (pos >= filp->f_dentry->d_inode->i_size)
//     //     return EOF;

//     offset = pos % private->bytes_per_clus; /* 簇内偏移 */
//     index = pos / private->bytes_per_clus;  /* 第几个簇 */

//     clus = filp->f_dentry->d_inode->i_ino; /* 起始簇 */
//     buffer = (unsigned char *)kmalloc(private->bytes_per_clus, 0);
//     assert(buffer != NULL);

//     for (i = 0; i < index; i++) clus = get_next_cluster(private, clus);

// next_clus:
//     sector = private->data_first_sector + (clus - 2) * private->sector_per_clus;
//     dev_read(sector, private->sector_per_clus, buffer);
//     p = (fat32_directory_t *)buffer + (offset / 32);

//     for (i = offset; i < private->bytes_per_clus; i += 32, p++, filp->f_pos += 32) {
//         if (p->DIR_Attr == ATTR_LONG_NAME)
//             continue;
//         if (p->DIR_Name[0] == 0xe5 || p->DIR_Name[0] == 0x00 || p->DIR_Name[0] == 0x05)
//             continue;
//         len = get_dname(name, p, buffer);
//         if (len > 0) {
//             filp->f_pos += 32;
//             goto founded;
//         }
//     }
//     clus = get_next_cluster(private, clus);
//     if (clus < 0x0ffffff7)
//         goto next_clus;
//     kfree(buffer);
//     return NULL;
// founded:
//     type = p->DIR_Attr == ATTR_DIRECTORY ? FS_ATTR_DIR : FS_ATTR_FILE;
//     size = p->DIR_Attr == ATTR_DIRECTORY ? private->bytes_per_clus : p->DIR_FileSize;
//     time = ((u64)p->DIR_WrtDate << 32) | p->DIR_WrtTime;
//     kfree(buffer);
//     return filldir(dirent, name, len, size, time, type);
// }

// struct file_operations fat32_file_operations = {
//     .lseek = lseek,
//     .read = read,
//     .write = write,
//     .open = open,
//     .release = release,
//     .readdir = fat32_readdir,
// };

static void *fat32_setsb(unsigned long lba, struct fs_entry *entry)
{
    DBR_t *       dbr;
    FSInfo_t *    fsinfo;
    superblock_t *sb;
    int           status;

    dbr = (DBR_t *)malloc(sizeof(DBR_t));
    assert(dbr != NULL);
    fsinfo = (FSInfo_t *)malloc(sizeof(FSInfo_t));
    assert(fsinfo != NULL);
    sb = (superblock_t *)malloc(sizeof(superblock_t));

    status = storage_read(1, lba, dbr);
    assert(status == 0 && dbr->BS_TrailSig == 0xaa55);

    status = storage_read(1, lba + dbr->BPB_FSInfo, fsinfo);
    assert(status == 0);

    sb->start_sector = lba;
    sb->sector_per_clus = dbr->BPB_SecPerClus;
    sb->bytes_per_clus = dbr->BPB_SecPerClus * dbr->BPB_BytesPerSec;
    sb->bytes_per_sector = dbr->BPB_BytesPerSec;
    sb->data_first_sector = lba + dbr->BPB_RsvdSecCnt + dbr->BPB_FATSz32 * dbr->BPB_NumFATs;
    sb->fat1_first_sector = lba + dbr->BPB_RsvdSecCnt;
    sb->sector_per_fat = dbr->BPB_FATSz32;
    sb->num_of_fats = dbr->BPB_NumFATs;
    sb->fsinfo_sector_infat = dbr->BPB_FSInfo;
    sb->bootsector_bk_infat = dbr->BPB_BkBootSec;
    sb->first_cluster = dbr->BPB_RootClus;
    sb->dbr = dbr;
    sb->fsinfo = fsinfo;

    entry->inode = dbr->BPB_RootClus;  // 以根目录的第一个簇的簇号作为ino
    entry->mode = 0xff;                // fat没有文件权限之分
    entry->pread = dbr->BPB_RootClus;  // 读取位置以簇为单位
    entry->pwrite = -1;                // 先不进行写
    entry->fsize = -1;

    return (void *)sb;
}

int fat32_lookup(const char *filename, struct fs_entry *p_entry, struct fs_entry *entry, void *ptr_sb)
{
    dir_t *        dir;
    superblock_t * sb;
    int            i, status;
    unsigned int   clus;
    unsigned long  sect;
    unsigned char *buffer;

    sb = (superblock_t *)ptr_sb;
    buffer = (unsigned char *)malloc(sb->bytes_per_clus);
    clus = p_entry->pread;
next_clus:
    sect = sb->data_first_sector + (clus - 2) * sb->sector_per_clus;
    status = storage_read(sb->sector_per_clus, sect, buffer);
    assert(status == 0);

    dir = (dir_t *)buffer;
    for (i = 0; i < sb->bytes_per_clus; i += 32, dir++) {
        if (dir->DIR_Attr == ATTR_LONG_NAME)
            continue;
        if (dir->DIR_Name[0] == 0xe5 || dir->DIR_Name[0] == 0x00 || dir->DIR_Name[0] == 0x05)
            continue;
        if (match_name(filename, dir, buffer))
            goto founded;
    }
    clus = getnextclus(sb, clus);
    if (clus < 0x0ffffff7)
        goto next_clus;
    free(buffer);
    return -1;

founded:
    entry->inode = (unsigned long)dir->DIR_FstClusHI << 16 | dir->DIR_FstClusLO;
    entry->pread = entry->inode;
    entry->pwrite = -1;
    entry->fsize = dir->DIR_FileSize;
    entry->mode = 0xff;
    return 0;
}

static ssize_t fat32_read(const struct fs_entry *entry, void *buf, loff_t pos, size_t size, void *ptr_sb)
{
    superblock_t *sb;
    unsigned long index, offset, clus, sect, cpysize;
    ssize_t       retval;
    char *        buffer;
    int           i, status;

    sb = (superblock_t *)ptr_sb;
    if (pos >= entry->fsize)
        return EOF;
    offset = pos % sb->bytes_per_clus;
    index = pos / sb->bytes_per_clus;
    clus = entry->pread; /* 文件的第一个簇 */
    buffer = (char *)malloc(sb->bytes_per_clus);
    assert(buffer != NULL);

    for (i = 0; i < index; i++) clus = getnextclus(sb, clus);

    if (pos + size >= entry->fsize)
        size = entry->fsize - pos;

    retval = 0;
    do {
        sect = sb->data_first_sector + (clus - 2) * sb->sector_per_clus;
        status = storage_read(sb->sector_per_clus, sect, buffer);
        assert(status == 0);

        cpysize = (sb->bytes_per_clus - offset) > size ? size : sb->bytes_per_clus - offset;
        memcpy(buf, buffer + offset, cpysize);

        buf += cpysize;
        size -= cpysize;
        offset = 0;
        retval += cpysize;
        clus = getnextclus(sb, clus);
    } while (size > 0);
    return retval;
}

static ssize_t fat32_write(const struct fs_entry *entry, void *buf, loff_t pos, size_t size, void *ptr_sb)
{
    return -1;
}

static struct fs_ops fat32_ops = {
    .fs_lookup = fat32_lookup,
    .fs_read = fat32_read,
    .fs_write = fat32_write,
    .fs_setsb = fat32_setsb,
};

int main(void)
{
    int status;

    status = run_fs(&fat32_ops);
    if (status != 0) {
        panic("fat32 run error\n");
    }
    while (1) nop();
    return 0;
}
