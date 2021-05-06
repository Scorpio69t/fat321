#include <boot/cpu.h>
#include <boot/io.h>
#include <boot/mptable.h>
#include <kernel/bugs.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/string.h>

int nr_cpu = 0;
int boot_apic_id = 0;

extern unsigned long apic_base;   /* defined in apic.c */
extern unsigned long ioapic_base; /* defined in apic.c */
extern unsigned char ioapicid;    /* defined in apic.c */

static unsigned char checksum(unsigned char *addr, unsigned long length)
{
    int i, sum;

    sum = 0;
    for (i = 0; i < length; i++) sum += addr[i];
    return sum;
}

static struct intel_mp_floating *scan_mp_table(unsigned long base, unsigned long length)
{
    void *addr, *end;

    addr = (void *)to_vir(base);
    end = addr + length;
    for (; addr < end; addr += sizeof(struct intel_mp_floating)) {
        if (strncmp((const char *)addr, "_MP_", 4) == 0 && checksum(addr, sizeof(struct intel_mp_floating)) == 0)
            return (struct intel_mp_floating *)addr;
    }
    return 0;
}

static struct intel_mp_floating *find_mp_table()
{
    struct intel_mp_floating *mp;

    if ((mp = scan_mp_table(0, 0x400)))
        return mp;
    if ((mp = scan_mp_table(0x9fc00, 0x400)))
        return mp;
    if ((mp = scan_mp_table(0xf0000, 0x10000)))
        return mp;
    return 0;
}

static struct mp_config_table *get_mp_config(struct intel_mp_floating *mp)
{
    struct mp_config_table *mpconfig;

    mpconfig = (struct mp_config_table *)to_vir(mp->mpf_physptr);
    if (strncmp((const char *)mpconfig, MPC_SIGNATURE, 4) != 0)
        return 0;
    if (mpconfig->mpc_spec != 1 && mpconfig->mpc_spec != 4)
        return 0;
    return mpconfig;
}

void mp_init(void)
{
    struct intel_mp_floating *mp;
    struct mp_config_table *mpconfig;
    struct mpc_config_processor *proc;
    struct mpc_config_ioapic *ioapic;
    void *addr, *end;

    if (!(mp = find_mp_table())) {
        panic("can't find mp table\n");
        return;
    }
    if (!(mpconfig = get_mp_config(mp))) {
        panic("get mp config failed\n");
        return;
    }
    apic_base = to_vir(mpconfig->mpc_lapic);
    for (addr = (void *)(mpconfig + 1), end = (void *)mpconfig + mpconfig->mpc_length; addr < end;) {
        switch (*(unsigned char *)addr) {
        case MP_PROCESSOR:
            proc = (struct mpc_config_processor *)addr;

            if (!(proc->mpc_cpuflag & CPU_ENABLED)) {
                printk(KERN_ERR "Have disabled cpu\n");
                continue;
            }

            if (nr_cpu < NR_CPUS) {
                cpu_info[nr_cpu].apicid = proc->mpc_apicid;
                nr_cpu++;
            }
            if (proc->mpc_cpuflag & CPU_BOOTPROC) {
                boot_apic_id = proc->mpc_apicid;
                printk("Boot cpu apicid %d\n", boot_apic_id);
            }
            addr += sizeof(struct mpc_config_processor);
            continue;
        case MP_IOAPIC:
            ioapic = (struct mpc_config_ioapic *)addr;
            ioapicid = ioapic->mpc_apicid;
            ioapic_base = to_vir(ioapic->mpc_apicaddr);
            addr += sizeof(struct mpc_config_ioapic);
            continue;
        case MP_BUS:
        case MP_INTSRC:
        case MP_LINTSRC:
            addr += 8;
            continue;
        default:
            panic("mp table type error\n");
            break;
        }
    }

    printk("mp_init: nr_cpus %d ioapicid %x\n", nr_cpu, ioapicid);
}
