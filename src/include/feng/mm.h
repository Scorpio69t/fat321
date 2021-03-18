#ifndef _FENG_MM_H_
#define _FENG_MM_H_

#include <boot/boot.h>

#define __KERNEL_OFFSET KERNEL_OFFSET
#define __USER_OFFSET   0x40000000

#ifndef __ASSEMBLY__
#include <feng/mmzone.h>
#include <feng/types.h>

#define __packed __attribute__((packed))

extern struct zone  mm_zones[MAX_NR_ZONES];
extern struct page *mem_map;

/* 页目录物理地址 */
#define PAGE_TABLE_PHY_ADDR 0x200000
#define KERNEL_BASE         __KERNEL_OFFSET
#define USER_BASE           0x00000000
/* 内存信息所在的地址，跟loader中的一致 */
#define MEM_INFO_ADDR 0xc0000600
/* 保存VESA的显示基址的地址，在loader中定义 */
#define VIDEO_BASE_ADDR   0xc0000510
#define VIDEO_XRESOLUTION 0xc0000514
#define VIDEO_YRESOLUTION 0xc0000518
#define VIDEO_MAP_ADDR    0xe0000000
#define VIDEO_BUF_SIZE    0x00400000
/* 标示minfo数组结束的魔数 */
#define MEM_INFO_END_MAGIC 0x3f3f3f3f

/* 页数组的起始地址 */
#define PAGE_ARRAY_ADDR 0xc0200100

/* 页属性 */
#define PF_RESERVE (1UL << 0) /* 保留页，操作系统不能使用 */
#define PF_KERNEL  (1UL << 1)
#define PF_USER    (1UL << 2)

void mm_init();

#define to_phy(address)        ((uint64)address - KERNEL_OFFSET)
#define to_vir(address)        ((uint64)address + KERNEL_OFFSET)
#define phy_ptr(type, address) ((type *)to_phy(address))
#define vir_ptr(type, address) ((type *)to_vir(address))

#endif

#endif
