#ifndef _BOOT_BOOT_H_
#define _BOOT_BOOT_H_

#define KERNEL_OFFSET 0xffff800000000000

#define KERNEL_CODE_DESC 0x08
#define KERNEL_DATA_DESC 0x10
#define USER_CODE_DESC   0x18
#define USER_DATA_DESC   0x20
#define TSS_DESC         0x30

#define to_phy(address) (address - KERNEL_OFFSET)
#define to_vir(address) (address + KERNEL_OFFSET)

#endif
