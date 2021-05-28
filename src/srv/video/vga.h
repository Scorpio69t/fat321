#ifndef _VIDEO_VGA_H_
#define _VIDEO_VGA_H_

#include <sys/kernel.h>
#include <sys/types.h>

#define ROW                25
#define COL                80
#define DEFAULT_VIDEO_BASE (KERNEL_OFFSET + 0xb8000)

#endif
