#ifndef _ALPHAZ_FAT32_H_
#define _ALPHAZ_FAT32_H_

#define __packed __attribute__((packed))

void fat32_init(void);

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

/* 硬盘分区表，适用于主引导分区(MBR) */
struct disk_partition_table
{
	unsigned char BS_Reserved[446];
	struct disk_partition_table_entry DPTE[4];
	unsigned short BS_TrailSig;
} __packed;

/* 引导扇区(DBR) */
struct fat32_boot_sector
{
    unsigned char   BS_jmpBoot[3];
	unsigned char   BS_OEMName[8];
	unsigned short  BPB_BytesPerSec;
	unsigned char   BPB_SecPerClus;
	unsigned short  BPB_RsvdSecCnt;
	unsigned char   BPB_NumFATs;
	unsigned short  BPB_RootEntCnt;
	unsigned short  BPB_TotSec16;
	unsigned char   BPB_Media;
	unsigned short  BPB_FATSz16;
	unsigned short  BPB_SecPerTrk;
	unsigned short  BPB_NumHeads;
	unsigned int    BPB_HiddSec;
	unsigned int    BPB_TotSec32;

	unsigned int    BPB_FATSz32;
	unsigned short  BPB_ExtFlags;
	unsigned short  BPB_FSVer;
	unsigned int    BPB_RootClus;
	unsigned short  BPB_FSInfo;
	unsigned short  BPB_BkBootSec;
	unsigned char   BPB_Reserved[12];

	unsigned char   BS_DrvNum;
	unsigned char   BS_Reserved1;
	unsigned char   BS_BootSig;
	unsigned int    BS_VolID;
	unsigned char   BS_VolLab[11];
	unsigned char   BS_FilSysType[8];

	unsigned char   BootCode[420];

	unsigned short  BS_TrailSig;
} __packed;

/* FAT32文件系统的FSInfo扇区结构
 * 为FAT32文件系统在计算和索引空闲簇号的过程提供参考值，其值并非实时更新
 */
struct fat32_fs_info
{
    unsigned int  FSI_LeadSig;
	unsigned char FSI_Reserved1[480];
	unsigned int  FSI_StrucSig;
	unsigned int  FSI_Free_Count;
	unsigned int  FSI_Nxt_Free;
	unsigned char FSI_Reserved2[12];
	unsigned int  FSI_TrailSig;
};

#endif
