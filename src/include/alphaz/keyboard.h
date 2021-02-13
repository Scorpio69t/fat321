#ifndef _ALPHAZ_KEYBOARD_H_
#define _ALPHAZ_KEYBOARD_H_

#include <alphaz/type.h>
#include <alphaz/fs.h>

void keyboard_init(void);

/* defined in arch/i386/driver/i8042 */
unsigned char read_scancode(void);
void __sys_reboot(void);
void __keyboard_init(void);

#endif
