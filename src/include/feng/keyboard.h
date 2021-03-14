#ifndef _FENG_KEYBOARD_H_
#define _FENG_KEYBOARD_H_

#include <feng/fs.h>
#include <feng/types.h>

void keyboard_init(void);

/* defined in arch/i386/driver/i8042 */
unsigned char read_scancode(void);
void          __sys_reboot(void);
void          __keyboard_init(void);

#endif
