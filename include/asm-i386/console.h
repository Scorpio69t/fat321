#ifndef _ASM_CONSOLE_H_
#define _ASM_CONSOLE_H_

#include <alphaz/type.h>
#include <alphaz/mm.h>

#define DEFAULT_VIDEO_BASE    (__KERNEL_OFFSET + 0xb8000)

unsigned short get_cursor(void);
void set_cursor(unsigned short cur);
int console_curl(int line);
inline void write_char(char c, unsigned char type, unsigned short cur);

#endif
