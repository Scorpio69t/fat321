#ifndef _ALPHAZ_FAT32_H_
#define _ALPHAZ_FAT32_H_

#define __packed __attribute__((packed))

void fat32_init(void);

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
typedef struct fat32_boot_sector fat32_boot_sector_t;

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
typedef struct fat32_fs_info fat32_fs_info_t;

#define	ATTR_READ_ONLY	(1 << 0)
#define ATTR_HIDDEN	    (1 << 1)
#define ATTR_SYSTEM	    (1 << 2)
#define ATTR_VOLUME_ID	(1 << 3)
#define ATTR_DIRECTORY	(1 << 4)
#define ATTR_ARCHIVE	(1 << 5)
#define ATTR_LONG_NAME	0x0f

struct fat32_directory
{
	unsigned char   DIR_Name[11];
	unsigned char   DIR_Attr;
	unsigned char   DIR_NTRes;
	unsigned char   DIR_CrtTimeTenth;
	unsigned short  DIR_CrtTime;
	unsigned short  DIR_CrtDate;
	unsigned short  DIR_LastAccDate;
	unsigned short  DIR_FstClusHI;
	unsigned short  DIR_WrtTime;
	unsigned short  DIR_WrtDate;
	unsigned short  DIR_FstClusLO;
	unsigned int    DIR_FileSize;
} __packed;
typedef struct fat32_directory fat32_directory_t;

#define LOWERCASE_BASE (8)
#define LOWERCASE_EXT (16)

struct fat32_long_directory
{
	unsigned char   LDIR_Ord;
	unsigned short  LDIR_Name1[5];
	unsigned char   LDIR_Attr;
	unsigned char   LDIR_Type;
	unsigned char   LDIR_Chksum;
	unsigned short  LDIR_Name2[6];
	unsigned short  LDIR_FstClusLO;
	unsigned short  LDIR_Name3[2];
} __packed;
typedef struct fat32_long_directory fat32_long_directory_t;

struct fat32_private_info {
	unsigned long start_sector;
	unsigned long sector_count;
	unsigned int sector_per_clus;
	unsigned int bytes_per_clus;
	unsigned int bytes_per_sector;
	unsigned long data_first_sector;
	unsigned long fat1_first_sector;
	unsigned long sector_per_fat;
	unsigned long num_of_fats;

	unsigned long fsinfo_sector_infat; /* fsinfo的起始位置，相对于文件系统 */
	unsigned long bootsector_bk_infat; /* bootsector备份位置 */

    unsigned long first_cluster;
	unsigned long dentry_location;
	unsigned long dentry_position;
	unsigned short creat_date;
	unsigned short creat_time;
	unsigned short write_date;
	unsigned short write_time;
};

#endif
