#ifndef _BOOT_SMP_H_
#define _BOOT_SMP_H_

#define SMP_BOOT_BASE 0x8000

#ifndef __ASSEMBLY__

void smp_init(void);

extern int current_boot_cpu;
extern int nr_cpu;
extern int boot_apic_id;

unsigned char smp_processor_id(void);

#endif /* __ASSEMBLY__ */

#endif
