#ifndef _ALPHAZ_COMPILER_H_
#define _ALPHAZ_COMPILER_H_

#define __noinline __attribute__((noinline))
#define __data __attribute__((section(".data")))
/**
 * 使用最多三个寄存器传参，其余参数使用栈传参。从左到右三个参数分别使用eax，edx，ecx
 */
#define __regparm3 __attribute__((regparm(3)))


#endif
