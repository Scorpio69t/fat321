#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/types.h>

struct dirent {
    ino_t d_ino;
    off_t d_off;            /* 在目录文件中的偏移 */
    unsigned long d_reclen; /* 当前记录长度 */
    unsigned char d_type;
    char d_name[256];
};

typedef struct {
    int __fd;
    char *__data;
    size_t __size;
    char *__entry;
} DIR;

#define DT_UNKNOWN  0 /* Unknown File Type */
#define DT_REG_FILE 1 /* Regular File*/
#define DT_DIR      2 /* Directory File*/
#define DT_CHRDEV   3 /* Character Device*/
#define DT_BLKDEV   4 /* Block Device*/
#define DT_FIFO     5 /* Buffer File*/
#define DT_SOCK     6 /* Socket File*/
#define DT_SYMLINK  7 /* Symbolic Link*/

#define DIR_DATA_SIZE 4096

DIR *opendir(const char *pathname);
struct dirent *readdir(DIR *dp);
int closedir(DIR *dp);

#endif
