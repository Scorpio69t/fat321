#ifndef _KERNEL_IPC_H_
#define _KERNEL_IPC_H_

#include <boot/cpu.h>
#include <kernel/types.h>

#define IPC_NOWAIT 0
#define IPC_INIT   1
#define IPC_VFS    2
#define IPC_DISK   3
#define IPC_KB     4
#define IPC_VIDEO  5
#define IPC_MM     6

#define IPC_INTR   0x0ffffffd
#define IPC_ALL    0x0ffffffe
#define IPC_KERNEL 0x0fffffff

#define MSG_READ  1
#define MSG_WRITE 2
#define MSG_BRK   32

/* the aborve are syscall type */
#define MSG_CFM  256
#define MSG_DISK 257
#define MSG_IRQ  258
#define MSG_INTR 259

typedef struct {
    int    fd;
    void * buf;
    size_t size;
} msg_read;

typedef struct {
    int    fd;
    void * buf;
    size_t size;
} msg_write;

typedef struct {
    unsigned char type;
/* type */
#define DISK_READ  0
#define DISK_WRITE 1
#define DISK_IDEN  3
    unsigned char nsect;
    unsigned long sector;
    void *        buf;
} msg_disk;

typedef struct {
#define IRQ_REGISTER   1
#define IRQ_UNREGISTER 2
    int type;
    int irq_no;
} msg_irq;

typedef struct {
#define INTR_OK 1
    int type;
} msg_intr;

typedef struct {
    unsigned long addr;
} msg_brk;

typedef struct {
    int src;
    union {
        int  type;
        long ret;
    };
    union {
        msg_read  m_read;
        msg_write m_write;
        msg_disk  m_disk;
        msg_irq   m_irq;
        msg_intr  m_intr;
        msg_brk   m_brk;
    };
} message;

long do_send(frame_t *regs, pid_t to, message *msg);
long do_recv(frame_t *regs, pid_t from, message *msg);
long do_sendrecv(frame_t *regs, pid_t to, message *msg);
long process_kernel_message(message *msg);

#endif
