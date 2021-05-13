#ifndef _SYS_BLKDEV_H_
#define _SYS_BLKDEV_H_

/* 硬盘分区表表项 */
struct mbr_dpte {
    unsigned char flags;
    unsigned char start_head;
    unsigned short start_sector : 6, start_cylinder : 10;
    unsigned char type;
    unsigned char end_head;
    unsigned short end_sector : 6, end_cylinder : 10;
    unsigned int start_LBA;
    unsigned int sectors_limit;
} __attribute__((packed));

/* 硬盘分区表，适用于主引导分区(MBR) */
struct mbr_sector {
    unsigned char reserved[446];
    struct mbr_dpte dpte[4];
    unsigned short magic;
} __attribute__((packed));

#define GPT_SIGNATURE      "EFI PART"
#define GPT_ROOT_TYPE_GUID "B02F4C03-F4A9-4297-9F98-D6E20949450C" /* root partition type guid */

typedef struct {
    uint32 Data1;
    uint16 Data2;
    uint16 Data3;
    uint8 Data4[8];
} __attribute__((packed)) guid_t;

struct gpt_header {
    char signature[8];
    uint32 revision;
    uint32 hsize;    /* header size in bytes */
    uint32 crc;      /* CRC32 of header */
    uint32 reserved; /* reserved, must be zero */
    uint64 current;  /* current LBA (location of this header copy) */
    uint64 backup;   /* backup LBA (location of the other header copy) */
    uint64 pstart;   /* first usable LBA for partitions */
    uint64 plast;    /* last usable LBA */
    guid_t guid;     /* disk guid */
    uint64 pentry;   /* starting LBA of array of partition entries */
    uint32 pnum;     /* number of partition entries in array */
    uint32 pensz;    /* size of a single partition entry(usually 0x80) */
    uint32 pcrc;     /* CRC32 of partition entries array */
    uint8 reserved1[420];
};

struct gpt_part_entry {
    guid_t type_guid;   /* partition type GUID */
    guid_t unique_guid; /* Unique partition GUID */
    uint64 first_lba;   /* first LBA */
    uint64 last_lba;    /* last LBA */
    uint64 attr;        /* Attribute flags */
    uint8 name[72];     /* partition name, UTF-16 */
};

#endif
