#ifndef _KERNEL_BLKDEV_H_
#define _KERNEL_BLKDEV_H_

/* 硬盘分区表表项 */
struct mbr_dpte {
    unsigned char  flags;
    unsigned char  start_head;
    unsigned short start_sector : 6, start_cylinder : 10;
    unsigned char  type;
    unsigned char  end_head;
    unsigned short end_sector : 6, end_cylinder : 10;
    unsigned int   start_LBA;
    unsigned int   sectors_limit;
} __attribute__((packed));

/* 硬盘分区表，适用于主引导分区(MBR) */
struct mbr_sector {
    unsigned char   reserved[446];
    struct mbr_dpte dpte[4];
    unsigned short  magic;
} __attribute__((packed));

#endif
