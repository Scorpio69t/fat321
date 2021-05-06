/**
 * ext2.h - ext2 file system definitions
 * For more information about the ext2 file system,
 * please refer to https://www.nongnu.org/ext2-doc/ext2.html
 */

#ifndef _EXT2_H_
#define _EXT2_H_

#include <sys/types.h>

#define EXT2_SUPER_MAGIC 0xEF53

struct super_block {
    uint32 s_inodes_count;
    uint32 s_blocks_count;
    uint32 s_r_blocks_count;
    uint32 s_free_blocks_count;
    uint32 s_free_inodes_count;
    uint32 s_first_data_block;
    uint32 s_log_block_size;
    uint32 s_log_frag_size;
    uint32 s_blocks_per_group;
    uint32 s_frags_per_group;
    uint32 s_inodes_per_group;
    uint32 s_mtime;
    uint32 s_wtime;
    uint16 s_mnt_count;
    uint16 s_max_mnt_count;
    uint16 s_magic;
    uint16 s_state;
    uint16 s_errors;
    uint16 s_minor_rev_level;
    uint32 s_lastcheck;
    uint32 s_checkinterval;
    uint32 s_creator_os;
    uint32 s_rev_level;
    uint16 s_def_resuid;
    uint16 s_def_resgid;

    /* EXT2_DYNAMIC_REV Specific */
    uint32 s_first_ino;
    uint16 s_inode_size;
    uint16 s_block_group_nr;
    uint32 s_feature_compat;
    uint32 s_feature_incompat;
    uint32 s_feature_ro_compat;
    uint8 s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    uint32 s_algo_bitmap;

    /* Performance Hints */
    uint8 s_prealloc_blocks;
    uint8 s_prealloc_dir_blocks;
    uint16 s_padding1;

    /* Journaling Support */
    uint8 s_journal_uuid[16];
    uint32 s_journal_inum;
    uint32 s_journal_dev;
    uint32 s_last_orphan;

    /* Directory Indexing Support */
    uint32 s_hash_seed[4];
    uint8 s_def_hash_version;
    uint8 s_reserved1[3];

    /* Other options */
    uint32 s_default_mount_options;
    uint32 s_first_meta_bg;
    uint8 s_reserved2[760];

    /* Private Date: This is NOT part of the standard definition */
    uint64 partition_offset;
    uint64 block_size;
    int nr_group;
    struct group_desc *group_desc_table;
};

struct group_desc {
    uint32 bg_block_bitmap;
    uint32 bg_inode_bitmap;
    uint32 bg_inode_table;
    uint16 bg_free_blocks_count;
    uint16 bg_free_inodes_count;
    uint16 bg_used_dirs_count;
    uint16 bg_pad;
    uint32 bg_reserved[3];
};

/* Defined Reserved Inodes */
#define EXT2_BAD_INO         1 /* bad blocks inode */
#define EXT2_ROOT_INO        2 /* root directory inode*/
#define EXT2_ACL_IDX_INO     3 /* ACL index inode (deprecated?)*/
#define EXT2_ACL_DATA_INO    4 /* ACL data inode (deprecated?)*/
#define EXT2_BOOT_LOADER_INO 5 /* boot loader inode*/
#define EXT2_UNDEL_DIR_INO   6 /* undelete directory inode*/

/**
 * Defined i_mode Values
 */
/* file format */
#define EXT2_S_IFSOCK 0xC000 /* socket */
#define EXT2_S_IFLNK  0xA000 /* symbolic link */
#define EXT2_S_IFREG  0x8000 /* regular file */
#define EXT2_S_IFBLK  0x6000 /* block device*/
#define EXT2_S_IFDIR  0x4000 /* directory*/
#define EXT2_S_IFCHR  0x2000 /* character device*/
#define EXT2_S_IFIFO  0x1000 /* fifo*/
/* process execution user/group override */
#define EXT2_S_ISUID 0x0800 /* Set process User ID */
#define EXT2_S_ISGID 0x0400 /* Set process Group ID */
#define EXT2_S_ISVTX 0x0200 /* sticky bit */
/* access rights */
#define EXT2_S_IRUSR 0x0100 /* user read */
#define EXT2_S_IWUSR 0x0080 /* user write*/
#define EXT2_S_IXUSR 0x0040 /* user execute*/
#define EXT2_S_IRGRP 0x0020 /* group read*/
#define EXT2_S_IWGRP 0x0010 /* group write*/
#define EXT2_S_IXGRP 0x0008 /* group execute*/
#define EXT2_S_IROTH 0x0004 /* others read*/
#define EXT2_S_IWOTH 0x0002 /* others write*/
#define EXT2_S_IXOTH 0x0001 /* others execute*/

struct inode {
    uint16 i_mode;
    uint16 i_uid;
    uint32 i_size;
    uint32 i_atime;
    uint32 i_ctime;
    uint32 i_mtime;
    uint32 i_dtime;
    uint16 i_gid;
    uint16 i_links_count;
    uint32 i_blocks;
    uint32 i_flags;
    union {
        struct {
            uint32 l_i_reserved1;
        } linux1;
    } osd1; /* OS dependent 1 */
    uint32 i_block[15];
    uint32 i_generation;
    uint32 i_file_acl;
    uint32 i_dir_acl;
    uint32 i_faddr;
    union {
        struct {
            uint8 l_i_frag;
            uint8 l_i_fsize;
            uint16 i_pad1;
            uint16 l_i_uid_high;
            uint16 l_i_gid_high;
            uint32 l_i_reserved2;
        } linux2;
    } osd2; /* OS dependent 2 */
};

#define EXT2_FT_UNKNOWN  0 /* Unknown File Type */
#define EXT2_FT_REG_FILE 1 /* Regular File*/
#define EXT2_FT_DIR      2 /* Directory File*/
#define EXT2_FT_CHRDEV   3 /* Character Device*/
#define EXT2_FT_BLKDEV   4 /* Block Device*/
#define EXT2_FT_FIFO     5 /* Buffer File*/
#define EXT2_FT_SOCK     6 /* Socket File*/
#define EXT2_FT_SYMLINK  7 /* Symbolic Link*/

struct linked_directory {
    uint32 inode;
    uint16 rec_len;
    uint8 name_len;
    uint8 file_type;
    char name[];
};

#endif
