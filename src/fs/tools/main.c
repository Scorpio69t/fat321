#include "../../include/alphaz/fat32.h"
#include "../../include/alphaz/blkdev.h"
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SECTOR_SIZE     512

disk_partition_table_t dpt;
fat32_boot_sector_t boot_sector;
fat32_fs_info_t     fs_info;

unsigned long first_data_sector;
unsigned long bytes_per_clus;
unsigned long first_fat1_sector, first_fat2_sector;

int fd;

void dev_read(unsigned long sector, unsigned long nsect, void *buf)
{
    off_t offset;
    ssize_t st;

    offset = lseek(fd, sector * SECTOR_SIZE, SEEK_SET);
    if (offset == -1) {
        printf("error: file offset can't be set\n");
    }

    st = read(fd, buf, nsect * SECTOR_SIZE);
    if (st == -1) {
        printf("error: file read error\n");
    } else if (st != nsect * SECTOR_SIZE) {
        printf("error: bytes is loss than needed\n");
    }
}

unsigned int get_next_cluster(unsigned int entry)
{
    unsigned int buf[128];
    memset(buf, 0, 512);
    dev_read(first_fat1_sector + (entry >> 7), 1, buf);
    return buf[entry & 0x7f] & 0x0fffffff;
}

/* 根据短目录项获取文件名, 返回文件名的长度
 * 传入begin是防止越界
 */
static int get_dname(char *buf, fat32_directory_t *de, void *begin)
{
    int len, i, j;
    static short tmp[512];

    j = 0;
    fat32_long_directory_t *lde = (fat32_long_directory_t *)de - 1;
    while ((void *)de >= begin && lde->LDIR_Attr == ATTR_LONG_NAME &&
           lde->LDIR_Ord != 0xe5) {
        for (i = 0; i < 5; i++)
            tmp[j++] = lde->LDIR_Name1[i];
        for (i = 0; i < 6; i++)
            tmp[j++] = lde->LDIR_Name2[i];
        for (i = 0; i < 2; i++)
            tmp[j++] = lde->LDIR_Name3[i];
        lde--;
    }
    for (len = 0; len < j; ) {
        buf[len] = (char)tmp[len];
        len++;
        if (!buf[len - 1]) break;
    }
    return len; /* 包含\0的长度 */
}

static int match_dentry(const char *name, int nl, fat32_directory_t *de, void *begin)
{
    static char tmp[512];
    int len;

    len = get_dname(tmp, de, begin);
    if (len <= 0)
        return 0;
    if (strncmp(tmp, name, nl < len ? nl : len))
        return 0;
    return 1;
}

fat32_directory_t * lookup(const char *name, int len, fat32_directory_t *dentry, int flags)
{
    fat32_directory_t *next_dentry, *p;
    unsigned int cluster;
    unsigned long sector;
    unsigned char *buf;
    char dname[128];
    int i, dlen;

    buf = (unsigned char *)malloc(bytes_per_clus);
    cluster = (dentry->DIR_FstClusHI << 16 | dentry->DIR_FstClusLO) & 0x0fffffff;
    next_cluster:
    sector = first_data_sector + (cluster - 2) * boot_sector.BPB_SecPerClus;
    dev_read(sector, boot_sector.BPB_SecPerClus, buf);

    p = (fat32_directory_t *)buf;
    for (i = 0; i < bytes_per_clus; i += 32, p++) {
        if (p->DIR_Attr == ATTR_LONG_NAME)
            continue;
        if (p->DIR_Name[0] == 0xe5 || p->DIR_Name[0] == 0x00 || p->DIR_Name[0] == 0x05)
            continue;
        if (match_dentry(name, len, p, buf)) {
            printf("%s %d %d\n", name, cluster, i);
            next_dentry = (fat32_directory_t *)malloc(sizeof(fat32_directory_t));
            *next_dentry = *p;
            free(buf);
            return next_dentry;
        }
    }
    cluster = get_next_cluster(cluster);
    if (cluster < 0x0ffffff7)
        goto next_cluster;
    free(buf);
    return NULL;
}

fat32_directory_t * path_walk(char *path, int flags)
{
    char *name, *p;
    int len;
    fat32_directory_t *parent, *next;

    p = path;
    while (*p == '/')
        p++;
    if (!*p)
        return NULL;

    parent = (fat32_directory_t *)malloc(sizeof(fat32_directory_t));
    parent->DIR_FstClusLO = boot_sector.BPB_RootClus & 0xffff;
    parent->DIR_FstClusHI = (boot_sector.BPB_RootClus >> 16) & 0x0fff;

    while (1) {
        name = p;
        len = 0;
        while (*p && *p != '/') {
            p++;
            len++;
        }

        next = lookup(name ,len, parent, flags);

        if (next == NULL) {
            free(parent);
            return NULL;
        }

        if (!*p) {
            break;
        }

        while (*p == '/')
            p++;
        if (!*p)
            break;

        free(parent);
        parent = next;
    }

    return next;
}

void blkdev_init(void)
{
    char *p;
    int i;
    fat32_directory_t *dentry = NULL;
    dev_read(0, 1, &dpt);
    // printf("dpt0 start LBA: %d\n", dpt.DPTE[0].start_LBA);
    dev_read(dpt.DPTE[0].start_LBA, 1, &boot_sector);
    //printf("%s %d %d ", boot_sector.BS_OEMName, (int)boot_sector.BPB_TotSec32, (int)boot_sector.BPB_SecPerClus);
    // for (p = (char *)boot_sector.BS_FilSysType, i = 0; i < 8; i++, p++)
        // printf("%c", *p);
    // printf("\n");
    dev_read(dpt.DPTE[0].start_LBA + boot_sector.BPB_FSInfo, 1, &fs_info);
    // printf("0x%x 0x%x 0x%x 0x%x\n", fs_info.FSI_LeadSig, fs_info.FSI_StrucSig, fs_info.FSI_Free_Count, fs_info.FSI_Nxt_Free);

    first_data_sector = dpt.DPTE[0].start_LBA + boot_sector.BPB_RsvdSecCnt + boot_sector.BPB_FATSz32 * boot_sector.BPB_NumFATs;
    first_fat1_sector = dpt.DPTE[0].start_LBA + boot_sector.BPB_RsvdSecCnt;
    first_fat2_sector = first_fat1_sector + boot_sector.BPB_FATSz32;
    bytes_per_clus = boot_sector.BPB_SecPerClus * boot_sector.BPB_BytesPerSec;
    // printf("%ld %ld %ld %ld\n", first_data_sector, first_fat1_sector, first_fat2_sector, bytes_per_clus);

    dentry = path_walk("/user/root", 0);
    if (dentry) {
        for (i = 0; i < 11; i++)
            printf("%c", dentry->DIR_Name[i]);
        printf("\n");
    } else {
        printf("error\n");
    }
}


int main(void)
{
    fd = open("../../alphaz.vhd", O_RDONLY);
    if (fd == -1) {
        printf("error: alphaz.vhd can't open\n");
        exit(-1);
    }
    blkdev_init();
    close(fd);
    return 0;
}
