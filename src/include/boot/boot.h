#ifndef _BOOT_BOOT_H_
#define _BOOT_BOOT_H_

#ifndef __ASSEMBLY__
#include <kernel/types.h>
#endif

#define KERNEL_OFFSET 0xffff800000000000

#define KERNEL_CODE_DESC 0x08
#define KERNEL_DATA_DESC 0x10
#define USER_CODE_DESC   0x1b
#define USER_DATA_DESC   0x23
#define TSS_DESC         0x30

#ifndef __ASSEMBLY__

#endif /* __ASSEMBLY__ */

#endif
