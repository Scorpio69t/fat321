#ifndef _KERNEL_ELF_H_
#define _KERNEL_ELF_H_

typedef unsigned long long Elf64_Addr;
typedef unsigned long long Elf64_Off;
typedef unsigned short Elf64_Half;
typedef unsigned int Elf64_Word;
typedef int Elf64_Sword;
typedef unsigned long long Elf64_Xword;
typedef long long Elf64_Sxword;

/* ELF ideltification index */
#define EI_MAG0       0  /* File identification */
#define EI_MAG1       1  /* File identification */
#define EI_MAG2       2  /* File identification */
#define EI_MAG3       3  /* File identification */
#define EI_CLASS      4  /* File class */
#define EI_DATA       5  /* Data encoding */
#define EI_VERSION    6  /* File version*/
#define EI_OSABI      7  /* Operating system/ABI identification*/
#define EI_ABIVERSION 8  /* ABI version*/
#define EI_PAD        9  /* Start of padding bytes*/
#define EI_NIDENT     16 /* Size of e_ident[]*/

/* ELF ideltification magic number */
#define ELFMAG0 0x7f /* e_ident[EI_MAG0] */
#define ELFMAG1 'E'  /* e_ident[EI_MAG1] */
#define ELFMAG2 'L'  /* e_ident[EI_MAG2] */
#define ELFMAG3 'F'  /* e_ident[EI_MAG3] */

/* ELF ideltification e_ident[EI_CLASS] */
#define LFCLASSNONE 0 /*	Invalid class*/
#define ELFCLASS32  1 /*32-bit objects*/
#define ELFCLASS64  2 /*64-bit objects*/

/* ELF file type */
#define ET_NONE   0      /* No file type */
#define ET_REL    1      /* Relocatable file*/
#define ET_EXEC   2      /* Executable file*/
#define ET_DYN    3      /* Shared object file*/
#define ET_CORE   4      /* Core file*/
#define ET_LOOS   0xfe00 /* Operating system-specific*/
#define ET_HIOS   0xfeff /* Operating system-specific*/
#define ET_LOPROC 0xff00 /*Processor-specific */
#define ET_HIPROC 0xffff /* Processor-specific */

/* the required architecture */
#define EM_X86_64 62 /* AMD x86-64 architecture */

typedef struct {
    unsigned char e_ident[EI_NIDENT]; /* ELF ideltification */
    Elf64_Half e_type;                /* file type */
    Elf64_Half e_machine;             /* required architecture */
    Elf64_Word e_version;             /* file version, always 1 */
    Elf64_Addr e_entry;               /* virtual address of entrty point */
    Elf64_Off e_phoff;                /* program header table's file offset in bytes */
    Elf64_Off e_shoff;                /* section header table's file offset in bytes */
    Elf64_Word e_flags;               /* processor-specific flags */
    Elf64_Half e_ehsize;              /* ELF header's size in bytes */
    Elf64_Half e_phentsize;           /* the size in bytes of one entry in the file's program header table */
    Elf64_Half e_phnum;               /* the number of entries in the program header table */
    Elf64_Half e_shentsize;           /* a section header's size in bytes */
    Elf64_Half e_shnum;               /* the number of entries in the section header table */
    Elf64_Half e_shstrndx;            /* the section header table index */
} Elf64_Ehdr;

/* segment type */
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7
#define PT_LOOS    0x60000000
#define PT_HIOS    0x6fffffff
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

/* Segment Flag Bits */
#define PF_X        0x1        /* Execute */
#define PF_W        0x2        /* Write */
#define PF_R        0x4        /* Read */
#define PF_MASKOS   0x0ff00000 /* Unspecified */
#define PF_MASKPROC 0xf0000000 /* Unspecified */

typedef struct {
    Elf64_Word p_type;    /* segment type */
    Elf64_Word p_flags;   /* segment flags */
    Elf64_Off p_offset;   /* the offset from the beginning of the file */
    Elf64_Addr p_vaddr;   /* virtual start address */
    Elf64_Addr p_paddr;   /* physical start addressing */
    Elf64_Xword p_filesz; /* size in the file */
    Elf64_Xword p_memsz;  /* size in the memory */
    Elf64_Xword p_align;
} Elf64_Phdr;

#endif
