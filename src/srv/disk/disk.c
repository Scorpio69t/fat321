/*
 * IDE硬盘驱动程序，当前驱动还和x86体系相关，耦合度很高，我想先处理整体功能，然后在对整体结构
 * 进行调整。
 */

#include "disk.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/blkdev.h>
#include <sys/io.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

static struct {
    unsigned long position;
    size_t size;
    void *buffer;
} request;

static unsigned short sector_per_drq;

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
        outnw(PORT_DISK0_DATA, buf, SECTOR_SIZE / 2);
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
        outnw(PORT_DISK0_DATA, buf, SECTOR_SIZE / 2);

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
        break;
    default:
        return -1;
        break;
    }
    return 0;
}

static int disk_read(void)
{
    static unsigned char buffer[SECTOR_SIZE];
    unsigned int copysz, offset;

    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        debug("read_handler: disk read error\n");
        return ERROR;
    } else {
        innw(PORT_DISK0_DATA, buffer, SECTOR_SIZE / 2);
    }

    offset = request.position & (SECTOR_SIZE - 1);
    copysz = (SECTOR_SIZE - offset) < request.size ? (SECTOR_SIZE - offset) : request.size;
    memcpy(request.buffer, buffer + offset, copysz);
    request.position += copysz;
    request.size -= copysz;
    request.buffer += copysz;
    if (request.size == 0)
        return DONE;
    return UNDONE;
}

static int disk_write(void)
{
    static unsigned char buffer[SECTOR_SIZE];
    unsigned int copysz;
    memset(buffer, 0x00, SECTOR_SIZE);

    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        debug("write_handler: disk write error\n");
        return ERROR;
    }

    if (request.size > SECTOR_SIZE) {
        request.size -= SECTOR_SIZE;
        request.buffer += SECTOR_SIZE;
    } else {
        return DONE;
    }

    copysz = request.size > SECTOR_SIZE ? SECTOR_SIZE : request.size;
    memcpy(buffer, request.buffer, copysz);

    while (!(inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ)) nop();
    outnw(PORT_DISK0_DATA, buffer, SECTOR_SIZE / 2);
    return UNDONE;
}

static int disk_identify(void)
{
    if (inb(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR) {
        debug("identify_handler: disk read error\n");
    } else {
        innw(PORT_DISK0_DATA, request.buffer, 256);
    }
    return DONE;
}

int init_disk(void)
{
    message msg;
    unsigned short buffer[512];

    outb(PORT_DISK0_ALT_STA_CTL, 0);
    outb(PORT_DISK0_ERR_FEATURE, 0);
    outb(PORT_DISK0_SECTOR_CNT, 0);
    outb(PORT_DISK0_SECTOR_LOW, 0);
    outb(PORT_DISK0_SECTOR_MID, 0);
    outb(PORT_DISK0_SECTOR_HIGH, 0);
    outb(PORT_DISK0_DEVICE, 0xe0); /* sector模式 */

    if (register_irq(0x2e) != 0) {
        panic("Disk register irq error\n");
        return -1;
    }

    request.buffer = buffer;
    do_request(IDEN_CMD, 0, 0, NULL);
    _recv(IPC_INTR, &msg);
    disk_identify();

    sector_per_drq = buffer[59];
    debug("Sector per drq: %d\n", sector_per_drq);

    if (!(buffer[49] & 0x0100) || buffer[83] & 0x0200) {
        panic("Disk not support LBA48\n");
        return -1;
    }
    return 0;
}

static int transfer(unsigned long pos, void *buf, size_t size, int write)
{
    unsigned long sector, nsect, pos_base, pos_high;
    message mess;
    int cmd, retval;

    if (write && pos % SECTOR_SIZE) {
        debug("Position must sector-aligned for write\n");
        return ERROR;
    }

    pos_base = lower_div(pos, SECTOR_SIZE) * SECTOR_SIZE;
    pos_high = upper_div(pos + size, SECTOR_SIZE) * SECTOR_SIZE;

    nsect = (pos_high - pos_base) / SECTOR_SIZE;
    if (nsect > sector_per_drq) {
        debug("transfer: to many sector %d\n", nsect);
        return ERROR;
    }

    sector = pos_base / SECTOR_SIZE;
    cmd = write ? LBA48_WRITE_CMD : LBA48_READ_CMD;

    request.buffer = buf;
    request.position = pos;
    request.size = size;
    do_request(cmd, nsect, sector, buf);

    do {
        if ((_recv(IPC_INTR, &mess)) != 0) {
            debug("transfer recive intr message failed\n");
            return ERROR;
        }
        retval = write ? disk_write() : disk_read();
    } while (retval == UNDONE);
    return retval;
}

static long disk_part(int part)
{
    long start_lba;
    unsigned long base, offset;
    struct gpt_header *header;
    struct gpt_part_entry *entry;
    void *buffer;

    if (!(header = (struct gpt_header *)malloc(SECTOR_SIZE)))
        goto failed;
    if (transfer(SECTOR_SIZE * 1, header, SECTOR_SIZE, 0) != DONE)
        goto failed;
    if (strncmp(header->signature, GPT_SIGNATURE, 8)) {
        panic("gpt signature error\n");
        goto failed;
    }

    if (!(buffer = malloc(SECTOR_SIZE * 2)))  // 分区条目可能跨两个扇区
        goto failed;

    base = (header->pentry + header->pensz * part / SECTOR_SIZE) * SECTOR_SIZE;
    offset = header->pensz * part % SECTOR_SIZE;

    if (transfer(base, buffer, SECTOR_SIZE * 2, 0) != DONE)
        goto failed;
    entry = (struct gpt_part_entry *)(buffer + offset);  // 第2项，需要重构
    start_lba = entry->first_lba;

    free(header);
    free(buffer);
    return start_lba * SECTOR_SIZE;
failed:
    free(header);
    free(buffer);
    return -1;
}

int main(int argc, char *argv[])
{
    unsigned long part_base;
    message mess;

    if (init_disk() == -1)
        goto fail;

    while (1) {
        if (_recv(IPC_ALL, &mess) != 0) {
            debug("disk recive message failed\n");
            continue;
        }
        switch (mess.type) {
        case MSG_BDEV_TRANSFER:
            mess.retval = transfer(mess.m_bdev_transfer.pos, mess.m_bdev_transfer.buffer, mess.m_bdev_transfer.size,
                                   mess.m_bdev_transfer.write);
            break;
        case MSG_BDEV_PART:
            if ((part_base = disk_part(mess.m_bdev_part.part)) < 0 ||
                transfer(part_base + mess.m_bdev_part.pos, mess.m_bdev_part.buffer, mess.m_bdev_part.size, 0) != DONE) {
                mess.retval = -1;
                break;
            }
            mess.retval = part_base;
            break;
        default:
            debug("disk: unknow msg, src %d type %x\n", mess.src, mess.type);
        }
        if (_send(mess.src, &mess) != 0) {
            debug("disk send message failed\n");
        }
    }
fail:
    debug("disk init error\n");
    while (1) nop();
}
