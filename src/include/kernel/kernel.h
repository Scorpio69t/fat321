#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_

#include <kernel/multiboot.h>
#include <kernel/types.h>
#include <stdarg.h>

#define KINFO_MEMMAP_SIZE 16
#define KINFO_MODULE_SIZE 8

typedef struct {
    uint64 addr;
    uint64 len;
} module_mmap_t;

typedef struct kernel_info {
    uint32                 mmap_size;
    multiboot_memory_map_t mmap[KINFO_MEMMAP_SIZE];

    uint32                 module_size;
    multiboot_tag_module_t module[KINFO_MODULE_SIZE];
    module_mmap_t          module_mmap[KINFO_MODULE_SIZE];
    uint64                 kernrl_start, kernel_end;         /* linear address of kernel */
    uint64                 global_pgd_start, global_pgd_end; /* global page table start and end */
    uint64                 mem_map_start, mem_map_end;       /* mem_map array start address and end */
    uint64                 mmio_start, mmio_end;
} kinfo_t;

extern kinfo_t kinfo;

/* printk打印前缀 */
#define KERN_EMERG   "<0>" /* system is unusable            */
#define KERN_ALERT   "<1>" /* action must be taken immediately    */
#define KERN_CRIT    "<2>" /* critical conditions            */
#define KERN_ERR     "<3>" /* error conditions            */
#define KERN_WARNING "<4>" /* warning conditions            */
#define KERN_NOTICE  "<5>" /* normal but significant condition    */
#define KERN_INFO    "<6>" /* informational            */
#define KERN_DEBUG   "<7>" /* debug-level messages            */

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printk(const char *fmt, ...);

#endif
