#ifndef _BOOT_SMP_H_
#define _BOOT_SMP_H_

#define SMP_BOOT_BASE 0x8000

#ifndef __ASSEMBLY__

void smp_init(void);

#endif /* __ASSEMBLY__ */

#endif
