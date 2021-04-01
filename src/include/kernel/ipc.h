#ifndef _COMM_IPC_H_
#define _COMM_IPC_H_

#include <kernel/types.h>

#define INIT_DEST  1
#define VFS_DEST   2
#define DISK_DEST  3
#define INPUT_DEST 4
#define TTY_DEST   5

#define MSG_READ 1
typedef struct {
    int    fd;
    void*  buf;
    size_t size;
} msg_read;

#define MSG_WRITE 2
typedef struct {
    int    fd;
    void*  buf;
    size_t size;
} msg_write;

typedef struct {
    int src;
    int type;
    union {
        msg_read  m_read;
        msg_write m_write;
    };

} message;

#endif
