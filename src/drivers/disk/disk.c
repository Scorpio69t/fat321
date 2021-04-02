/*
 * IDE硬盘驱动程序，当前驱动还和x86体系相关，耦合度很高，我想先处理整体功能，然后在对整体结构
 * 进行调整。
 */

#include "disk.h"

#include <stdio.h>
#include <sys/io.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

static struct {
    int secsz; /* 扇区大小 */
    int lba48; /* 是否支持LBA48 */
} dinfo;

static struct {
    int   nsect;
    void *buf;
} req;

static int do_request(int cmd, unsigned short nsect, unsigned long sector, void *buf)
{
    while (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_BUSY) nop();
    switch (cmd) {
    case LBA28_WRITE_CMD:
        outb(PORT_DISK0_DEVICE, 0xe0 | ((sector >> 24) & 0x0f));
        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, nsect);
        outb(PORT_DISK0_SECTOR_LOW, sector & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 16) & 0xff);
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY)) nop();
        outb(PORT_DISK0_STATUS_CMD, LBA28_WRITE_CMD);
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ)) nop();
        outnb(PORT_DISK0_DATA, buf, dinfo.secsz);
        break;
    case LBA28_READ_CMD:
        outb(PORT_DISK0_DEVICE, 0xe0 | ((sector >> 24) & 0x0f));
        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, nsect);
        outb(PORT_DISK0_SECTOR_LOW, sector & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 16) & 0xff);
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY)) nop();
        outb(PORT_DISK0_STATUS_CMD, LBA28_READ_CMD);
        break;
    case LBA48_WRITE_CMD:
        outb(PORT_DISK0_DEVICE, 0x40);

        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, (nsect >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_LOW, (sector >> 24) & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 32) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 40) & 0xff);

        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, nsect & 0xff);
        outb(PORT_DISK0_SECTOR_LOW, sector & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 16) & 0xff);

        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY)) nop();
        outb(PORT_DISK0_STATUS_CMD, LBA48_WRITE_CMD);
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ)) nop();
        outnb(PORT_DISK0_DATA, buf, dinfo.secsz);

        break;

    case LBA48_READ_CMD:
        outb(PORT_DISK0_DEVICE, 0x40);

        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, (nsect >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_LOW, (sector >> 24) & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 32) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 40) & 0xff);

        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, nsect & 0xff);
        outb(PORT_DISK0_SECTOR_LOW, sector & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 16) & 0xff);

        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY)) nop();
        outb(PORT_DISK0_STATUS_CMD, LBA48_READ_CMD);

        break;
    case IDEN_CMD:
        outb(PORT_DISK0_DEVICE, 0xe0 | ((sector >> 24) & 0x0f));
        outb(PORT_DISK0_ERR_FEATURE, 0);
        outb(PORT_DISK0_SECTOR_CNT, nsect);
        outb(PORT_DISK0_SECTOR_LOW, sector & 0xff);
        outb(PORT_DISK0_SECTOR_MID, (sector >> 8) & 0xff);
        outb(PORT_DISK0_SECTOR_HIGH, (sector >> 16) & 0xff);
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY)) nop();
        outb(PORT_DISK0_STATUS_CMD, cmd);
    default:
        return -1;
        break;
    }
    return 0;
}

static int disk_read(void)
{
    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        dprintf("read_handler: disk read error\n");
    } else {
        innb(PORT_DISK0_DATA, req.buf, dinfo.secsz);
    }
    if (--req.nsect) {
        /* 若当前请求未读完，则继续等待下一个中断进行处理 */
        req.buf += dinfo.secsz;
        return UNDONE;
    }
    return DONE;
}

static int disk_write(void)
{
    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        dprintf("write_handler: disk write error\n");
    }

    if (--req.nsect) {
        req.buf += dinfo.secsz;
        while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ)) nop();
        outnb(PORT_DISK0_DATA, req.buf, dinfo.secsz);
        return UNDONE;
    }
    return DONE;
}

static int disk_iden(void)
{
    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        dprintf("identify_handler: disk read error\n");
    } else {
        innb(PORT_DISK0_DATA, req.buf, dinfo.secsz);
    }
    return DONE;
}

unsigned short buf[256];

int init_disk(void)
{
    message msg;

    outb(PORT_DISK0_ALT_STA_CTL, 0);
    outb(PORT_DISK0_ERR_FEATURE, 0);
    outb(PORT_DISK0_SECTOR_CNT, 0);
    outb(PORT_DISK0_SECTOR_LOW, 0);
    outb(PORT_DISK0_SECTOR_MID, 0);
    outb(PORT_DISK0_SECTOR_HIGH, 0);
    outb(PORT_DISK0_DEVICE, 0xe0); /* sector模式 */

    msg.type = MSG_IRQ;
    msg.m_irq.type = IRQ_REGISTER;
    msg.m_irq.irq_no = 0x2e;
    sys_send(IPC_INTR, &msg);

    req.buf = buf;
    do_request(IDEN_CMD, 0, 0, NULL);
    sys_recv(IPC_INTR, &msg);
    disk_iden();

    if (msg.type == MSG_CFM && msg.m_cfm.type == CFM_OK)
        ;
    else {
        dprintf("msg error %d %d\n", msg.type, msg.m_cfm.type);
        return -1;
    }

    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
        dprintf("disk read error\n");
    else
        innb(PORT_DISK0_DATA, buf, 512);

    if (!(buf[49] & 0x0100)) {
        dprintf("unsupport LBA");
        return -1;
    }
    if (buf[83] & 0x0200)
        dinfo.lba48 = 1;

    dinfo.secsz = 512;
    return 0;
}

static int to_disk_cmd(int type)
{
    int cmd;
    switch (type) {
    case DISK_READ:
        if (dinfo.lba48)
            cmd = LBA48_READ_CMD;
        else
            cmd = LBA28_READ_CMD;
        break;
    case DISK_WRITE:
        if (dinfo.lba48)
            cmd = LBA48_WRITE_CMD;
        else
            cmd = LBA28_WRITE_CMD;
        break;
    case DISK_IDEN:
        cmd = IDEN_CMD;
        break;
    default:
        cmd = -1;
        break;
    }
    return cmd;
}

static int (*handler[])(void) = {
    [DISK_READ] = disk_read,
    [DISK_WRITE] = disk_write,
    [DISK_IDEN] = disk_iden,
};

int main(int argc, char *argv[])
{
    if (init_disk() == -1)
        goto fail;

    dprintf("LBA48: %d, secsz %d\n", dinfo.lba48, dinfo.secsz);

    int     type, cmd;
    message msg;
    while (1) {
        sys_recv(IPC_BOTH, &msg);
        if (msg.type != MSG_DISK) {
            dprintf("disk: unknow msg\n");
            continue;
        }
        req.nsect = msg.m_disk.nsect;
        req.buf = msg.m_disk.buf;
        type = msg.m_disk.type;
        if ((cmd = to_disk_cmd(msg.m_disk.type)) == -1) {
            dprintf("disk: unknow msg_disk type\n");
            continue;
        }
        do_request(cmd, msg.m_disk.nsect, msg.m_disk.sector, buf);

        while (1) {
            sys_recv(IPC_INIT, &msg);
            if (msg.type == MSG_CFM && msg.m_cfm.type == CFM_OK) {
                if (handler[type] == DONE)
                    break;
            } else {
                dprintf("msg confim error\n");
                break;
            }
        }
    }
fail:
    dprintf("disk init error\n");
    while (1) nop();
}
