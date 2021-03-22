#ifndef _BOOT_BOOT_H_
#define _BOOT_BOOT_H_

#ifndef __ASSEMBLY__
#include <feng/types.h>
#endif

#define KERNEL_OFFSET 0xffff800000000000

#define KERNEL_CODE_DESC 0x08
#define KERNEL_DATA_DESC 0x10
#define USER_CODE_DESC   0x1b
#define USER_DATA_DESC   0x23
#define TSS_DESC         0x30

#ifndef __ASSEMBLY__

/* boot_info is defined in head.S, it's 32-bit physical
 * address of the Multiboot2 information structure
 */
extern uint32 boot_info;

#define MEMINFO_SIZE 32
struct meminfo_struct {
    uint64 address;
    uint64 limit;
    uint8  type;
};

extern struct meminfo_struct meminfo[MEMINFO_SIZE];

void boot_init(void);
int  check_meminfo_end(struct meminfo_struct *);
int  check_memarea_available(struct meminfo_struct *);

#endif /* __ASSEMBLY__ */

#endif
