#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <sys/types.h>

struct stat {
    mode_t st_mode;
    unsigned int st_ino;
    off_t st_size;
    unsigned long st_atime;
    unsigned long st_mtime;
    unsigned long st_ctime;
};

/* stat mode */
#define S_IFSOCK 0xC000 /* socket */
#define S_IFLNK  0xA000 /* symbolic link */
#define S_IFREG  0x8000 /* regular file */
#define S_IFBLK  0x6000 /* block device*/
#define S_IFDIR  0x4000 /* directory*/
#define S_IFCHR  0x2000 /* character device*/
#define S_IFIFO  0x1000 /* fifo*/
/* process execution user/group override */
#define S_ISUID 0x0800 /* Set process User ID */
#define S_ISGID 0x0400 /* Set process Group ID */
#define S_ISVTX 0x0200 /* sticky bit */
/* access rights */
#define S_IRUSR 0x0100 /* user read */
#define S_IWUSR 0x0080 /* user write*/
#define S_IXUSR 0x0040 /* user execute*/
#define S_IRGRP 0x0020 /* group read*/
#define S_IWGRP 0x0010 /* group write*/
#define S_IXGRP 0x0008 /* group execute*/
#define S_IROTH 0x0004 /* others read*/
#define S_IWOTH 0x0002 /* others write*/
#define S_IXOTH 0x0001 /* others execute*/

int stat(const char *pathname, struct stat *buf);

#endif
