#ifndef _ALPHAZ_BLKDEV_H_
#define _ALPHAZ_BLKDEV_H_

#include <asm/disk.h>

#define __packed __attribute__((packed))

#define BLK_READ    ATA_READ_CMD
#define BLK_WRITE   ATA_WRITE_CMD
#define BLK_IDEN    GET_IDENTIFY_DISK_CMD

/* 硬盘分区表表项 */
struct disk_partition_table_entry
{
	unsigned char   flags;
	unsigned char   start_head;
	unsigned short  start_sector:6, start_cylinder:10;
	unsigned char   type;
	unsigned char   end_head;
	unsigned short  end_sector:6, end_cylinder:10;
	unsigned int    start_LBA;
	unsigned int    sectors_limit;
} __packed;
typedef struct disk_partition_table_entry disk_partition_table_entry_t;

/* 硬盘分区表，适用于主引导分区(MBR) */
struct disk_partition_table
{
	unsigned char BS_Reserved[446];
	struct disk_partition_table_entry DPTE[4];
	unsigned short BS_TrailSig;
} __packed;
typedef struct disk_partition_table disk_partition_table_t;

#endif
