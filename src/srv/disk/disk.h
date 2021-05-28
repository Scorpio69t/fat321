#ifndef _BOOT_DISK_H_
#define _BOOT_DISK_H_

#define PORT_DISK0_DATA        0x1f0
#define PORT_DISK0_ERR_FEATURE 0x1f1
#define PORT_DISK0_SECTOR_CNT  0x1f2
#define PORT_DISK0_SECTOR_LOW  0x1f3
#define PORT_DISK0_SECTOR_MID  0x1f4
#define PORT_DISK0_SECTOR_HIGH 0x1f5
#define PORT_DISK0_DEVICE      0x1f6
#define PORT_DISK0_STATUS_CMD  0x1f7

#define PORT_DISK0_ALT_STA_CTL 0x3f6 /* 主控制器控制端口 */

#define DISK_STATUS_BUSY  (1 << 7)
#define DISK_STATUS_READY (1 << 6)
#define DISK_STATUS_SEEK  (1 << 4)
#define DISK_STATUS_REQ   (1 << 3)
#define DISK_STATUS_ERROR (1 << 0)

#define LBA28_READ_CMD  0x20 /* 28位LBA读 */
#define LBA28_WRITE_CMD 0x30 /* 28位LBA写 */
#define LBA48_READ_CMD  0x24 /* 48位LBA读 */
#define LBA48_WRITE_CMD 0x34 /* 48位LBA写 */
#define IDEN_CMD        0xEC

#define SECTOR_SIZE 512

#define ERROR  -1
#define DONE   0
#define UNDONE 1

#endif
