#ifndef _ASM_BUG_H_
#define _ASM_BUG_H_

#include <alphaz/compiler.h>

/* 屏幕显示位置 defined in bug.c */
extern int disp_pos;

void bug_init();

void disp_str(char *);

void disp_int(int);

void delay(int);

void nop(void);

/* 用于测试的进程调度的函数 */
void TestA();
void hlt();
void TestB();

#endif
