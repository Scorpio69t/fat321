#ifndef _SYS_IPC_H_
#define _SYS_IPC_H_

#include <sys/types.h>

#define IPC_NOWAIT 0
#define IPC_INIT   1
#define IPC_VFS    2
#define IPC_DISK   3
#define IPC_INPUT  4
#define IPC_TTY    5
#define IPC_INTR   6
#define IPC_BOTH   7

#define MSG_READ  1
#define MSG_WRITE 2
/* the aborve are syscall type */
#define MSG_CFM  256
#define MSG_DISK 257
#define MSG_IRQ  258

typedef struct {
    int    fd;
    void*  buf;
    size_t size;
} msg_read;

typedef struct {
    int    fd;
    void*  buf;
    size_t size;
} msg_write;

typedef struct {
#define CFM_OK    0
#define CFM_ERROR 1
    int type;
    int errno;
} msg_cfm;

typedef struct {
    unsigned char type;
/* type */
#define DISK_READ  0
#define DISK_WRITE 1
#define DISK_IDEN  3
    unsigned char nsect;
    unsigned long sector;
    void*         buf;
} msg_disk;

typedef struct {
#define IRQ_REGISTER   1
#define IRQ_UNREGISTER 2
    int type;
    int irq_no;
} msg_irq;

typedef struct {
    int src;
    int type;
    union {
        msg_read  m_read;
        msg_write m_write;
        msg_cfm   m_cfm;
        msg_disk  m_disk;
        msg_irq   m_irq;
    };

} message;

#endif
