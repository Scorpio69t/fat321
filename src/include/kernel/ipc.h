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
#define IPC_EXT2   7

#define IPC_SIGNAL 0x0ffffffc
#define IPC_INTR   0x0ffffffd
#define IPC_ALL    0x0ffffffe
#define IPC_KERNEL 0x0fffffff

#define MSG_READ   1
#define MSG_WRITE  2
#define MSG_OPEN   3
#define MSG_CLOSE  4
#define MSG_FORK   5 /* no message struct */
#define MSG_EXECVE 6
#define MSG_LSEEK  7
#define MSG_EXIT   8
#define MSG_WAIT   9
#define MSG_BRK    10
#define MSG_GETPID 11 /* no message struct */
#define MSG_GETCWD 12
#define MSG_CHDIR  13

/* the aborve are syscall type */
#define MSG_CFM      256
#define MSG_DISK     257
#define MSG_IRQ      258
#define MSG_INTR     259
#define MSG_FSMNT    260
#define MSG_FSREAD   261
#define MSG_FSWRITE  262
#define MSG_FSLOOKUP 263
#define MSG_COPYFS   264
#define MSG_FREEFS   265 /* no message struct */
#define MSG_EXECFS   266 /* no message struct */
#define MSG_KMAP     267

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
    char * filepath;
    int    oflag;
    mode_t mode;
} msg_open;

typedef struct {
    int fd;
} msg_close;

typedef struct {
    int   fd;
    off_t offset;
    int   whence;
} msg_lseek;

typedef struct {
    const char * pathname;
    char *const *argv;
    char *const *envp;
} msg_execve;

typedef struct {
    int status;
} msg_exit;

typedef struct {
    int statloc;
} msg_wait;

typedef struct {
    unsigned long addr;
} msg_brk;

typedef struct {
    char * buf;
    size_t size;
} msg_getcwd;

typedef struct {
    const char *pathname;
} msg_chdir;

/* the aborve are syscall message */

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
#define FSMNT_STEP1 1
#define FSMNT_STEP2 2
    int type;
    /* step 1 */
    int systemid;
    /* step 2 */
    unsigned long inode;
    loff_t        pread;
    loff_t        pwrite;
    mode_t        mode;
    size_t        fsize;
} msg_fsmnt;

typedef struct {
    unsigned long inode;
    size_t        fsize;
    loff_t        pread;
    loff_t        offset;
    void *        buf;
    size_t        size;
} msg_fsread;

typedef struct {
    unsigned long inode;
    size_t        fsize;
    loff_t        pwrite;
    loff_t        offset;
    void *        buf;
    size_t        size;
} msg_fswrite;

typedef struct {
    unsigned long p_inode;
    loff_t        p_pread;
    char *        filename;

    /* retval */
    unsigned long inode;
    loff_t        pread;
    loff_t        pwrite;
    size_t        fsize;
    mode_t        mode;
} msg_fslookup;

typedef struct {
    pid_t pid; /* child pid */
} msg_copyfs;

typedef struct {
    void *addr1;
    void *addr2;
    void *addr3;
} msg_kmap;

typedef struct {
    int src;
    union {
        int  type;
        long retval;
    };
    union {
        msg_read   m_read;
        msg_write  m_write;
        msg_open   m_open;
        msg_close  m_close;
        msg_lseek  m_lseek;
        msg_execve m_execve;
        msg_exit   m_exit;
        msg_wait   m_wait;
        msg_brk    m_brk;
        msg_getcwd m_getcwd;
        msg_chdir  m_chdir;

        msg_disk     m_disk;
        msg_irq      m_irq;
        msg_intr     m_intr;
        msg_fsmnt    m_fsmnt;
        msg_fsread   m_fsread;
        msg_fswrite  m_fswrite;
        msg_fslookup m_fslookup;
        msg_copyfs   m_copyfs;
        msg_kmap     m_kmap;
    };
} message;

long do_send(frame_t *regs, pid_t to, message *msg);
long do_recv(frame_t *regs, pid_t from, message *msg);
long do_sendrecv(frame_t *regs, pid_t to, message *msg);
long process_mm_message(frame_t *regs, message *msg);
long process_kernel_message(frame_t *regs, message *msg);
long ipc_kmap(message *m);

#endif
