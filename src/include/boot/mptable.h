#ifndef _BOOT_MPTABLE_H_
#define _BOOT_MPTABLE_H_

void mp_init(void);

/*
 * The following definition comes from the linux kernel code
 */

struct intel_mp_floating {
    char          mpf_signature[4];  /* "_MP_" 			*/
    unsigned int  mpf_physptr;       /* Configuration table address	*/
    unsigned char mpf_length;        /* Our length (paragraphs)	*/
    unsigned char mpf_specification; /* Specification version	*/
    unsigned char mpf_checksum;      /* Checksum (makes sum 0)	*/
    unsigned char mpf_feature1;      /* Standard or configuration ? 	*/
    unsigned char mpf_imcrp;         /* Bit7 set for IMCR|PIC	*/
    unsigned char mpf_feature3;      /* Unused (0)			*/
    unsigned char mpf_feature4;      /* Unused (0)			*/
    unsigned char mpf_feature5;      /* Unused (0)			*/
};

#define MP_PROCESSOR 0
#define MP_BUS       1
#define MP_IOAPIC    2
#define MP_INTSRC    3
#define MP_LINTSRC   4

struct mp_config_table {
    char mpc_signature[4];
#define MPC_SIGNATURE "PCMP"
    unsigned short mpc_length; /* Size of table */
    char           mpc_spec;   /* 0x01 */
    char           mpc_checksum;
    char           mpc_oem[8];
    char           mpc_productid[12];
    unsigned int   mpc_oemptr;  /* 0 if not present */
    unsigned short mpc_oemsize; /* 0 if not present */
    unsigned short mpc_oemcount;
    unsigned int   mpc_lapic; /* APIC address */
    unsigned int   reserved;
};

struct mpc_config_processor {
    unsigned char mpc_type;
    unsigned char mpc_apicid;  /* Local APIC number */
    unsigned char mpc_apicver; /* Its versions */
    unsigned char mpc_cpuflag;
#define CPU_ENABLED       1 /* Processor is available */
#define CPU_BOOTPROCESSOR 2 /* Processor is the BP */
    unsigned int mpc_cpufeature;
#define CPU_STEPPING_MASK 0x0F
#define CPU_MODEL_MASK    0xF0
#define CPU_FAMILY_MASK   0xF00
    unsigned int mpc_featureflag; /* CPUID feature value */
    unsigned int mpc_reserved[2];
};

struct mpc_config_ioapic {
    unsigned char mpc_type;
    unsigned char mpc_apicid;
    unsigned char mpc_apicver;
    unsigned char mpc_flags;
#define MPC_APIC_USABLE 0x01
    unsigned int mpc_apicaddr;
};

#endif
