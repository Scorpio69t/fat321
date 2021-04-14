#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <sys/types.h>

pid_t fork(void);

#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */

ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, void *buf, size_t nbytes);
int     close(int fd);
off_t   lseek(int fd, off_t offset, int whence);

#endif
