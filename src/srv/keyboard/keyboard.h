#ifndef _KERNEL_KEYBOARD_H_
#define _KERNEL_KEYBOARD_H_

#include <sys/types.h>

#define NR_SCAN_CODES 0x80
#define MAP_COLS      2
#define FLAG_BREAK    0x80

/* Keymap for US MF-2 keyboard. */
unsigned char keymap[NR_SCAN_CODES * MAP_COLS] = {
    /* scan-code			!Shift		Shift */
    /* 0x00 - none		*/ 0,    0,
    /* 0x01 - ESC		*/ 0,    0,
    /* 0x02 - '1'		*/ '1',  '!',
    /* 0x03 - '2'		*/ '2',  '@',
    /* 0x04 - '3'		*/ '3',  '#',
    /* 0x05 - '4'		*/ '4',  '$',
    /* 0x06 - '5'		*/ '5',  '%',
    /* 0x07 - '6'		*/ '6',  '^',
    /* 0x08 - '7'		*/ '7',  '&',
    /* 0x09 - '8'		*/ '8',  '*',
    /* 0x0A - '9'		*/ '9',  '(',
    /* 0x0B - '0'		*/ '0',  ')',
    /* 0x0C - '-'		*/ '-',  '_',
    /* 0x0D - '='		*/ '=',  '+',
    /* 0x0E - BS		*/ '\b', 0,
    /* 0x0F - TAB		*/ '\t', 0,
    /* 0x10 - 'q'		*/ 'q',  'Q',
    /* 0x11 - 'w'		*/ 'w',  'W',
    /* 0x12 - 'e'		*/ 'e',  'E',
    /* 0x13 - 'r'		*/ 'r',  'R',
    /* 0x14 - 't'		*/ 't',  'T',
    /* 0x15 - 'y'		*/ 'y',  'Y',
    /* 0x16 - 'u'		*/ 'u',  'U',
    /* 0x17 - 'i'		*/ 'i',  'I',
    /* 0x18 - 'o'		*/ 'o',  'O',
    /* 0x19 - 'p'		*/ 'p',  'P',
    /* 0x1A - '['		*/ '[',  '{',
    /* 0x1B - ']'		*/ ']',  '}',
    /* 0x1C - CR/LF		*/ '\n', 0,
    /* 0x1D - l. Ctrl	*/ 0x1d, 0x1d,
    /* 0x1E - 'a'		*/ 'a',  'A',
    /* 0x1F - 's'		*/ 's',  'S',
    /* 0x20 - 'd'		*/ 'd',  'D',
    /* 0x21 - 'f'		*/ 'f',  'F',
    /* 0x22 - 'g'		*/ 'g',  'G',
    /* 0x23 - 'h'		*/ 'h',  'H',
    /* 0x24 - 'j'		*/ 'j',  'J',
    /* 0x25 - 'k'		*/ 'k',  'K',
    /* 0x26 - 'l'		*/ 'l',  'L',
    /* 0x27 - ';'		*/ ';',  ':',
    /* 0x28 - '\''		*/ '\'', '"',
    /* 0x29 - '`'		*/ '`',  '~',
    /* 0x2A - l. SHIFT	*/ 0x2a, 0x2a,
    /* 0x2B - '\'		*/ '\\', '|',
    /* 0x2C - 'z'		*/ 'z',  'Z',
    /* 0x2D - 'x'		*/ 'x',  'X',
    /* 0x2E - 'c'		*/ 'c',  'C',
    /* 0x2F - 'v'		*/ 'v',  'V',
    /* 0x30 - 'b'		*/ 'b',  'B',
    /* 0x31 - 'n'		*/ 'n',  'N',
    /* 0x32 - 'm'		*/ 'm',  'M',
    /* 0x33 - ','		*/ ',',  '<',
    /* 0x34 - '.'		*/ '.',  '>',
    /* 0x35 - '/'		*/ '/',  '?',
    /* 0x36 - r. SHIFT	*/ 0x36, 0x36,
    /* 0x37 - '*'		*/ '*',  '*',
    /* 0x38 - ALT		*/ 0x38, 0x38,
    /* 0x39 - ' '		*/ ' ',  ' ',
    /* 0x3A - CapsLock	*/ 0,    0,
    /* 0x3B - F1		*/ 0,    0,
    /* 0x3C - F2		*/ 0,    0,
    /* 0x3D - F3		*/ 0,    0,
    /* 0x3E - F4		*/ 0,    0,
    /* 0x3F - F5		*/ 0,    0,
    /* 0x40 - F6		*/ 0,    0,
    /* 0x41 - F7		*/ 0,    0,
    /* 0x42 - F8		*/ 0,    0,
    /* 0x43 - F9		*/ 0,    0,
    /* 0x44 - F10		*/ 0,    0,
    /* 0x45 - NumLock	*/ 0,    0,
    /* 0x46 - ScrLock	*/ 0,    0,
    /* 0x47 - Home		*/ '7',  0,
    /* 0x48 - CurUp		*/ '8',  0,
    /* 0x49 - PgUp		*/ '9',  0,
    /* 0x4A - '-'		*/ '-',  0,
    /* 0x4B - Left		*/ '4',  0,
    /* 0x4C - MID		*/ '5',  0,
    /* 0x4D - Right		*/ '6',  0,
    /* 0x4E - '+'		*/ '+',  0,
    /* 0x4F - End		*/ '1',  0,
    /* 0x50 - Down		*/ '2',  0,
    /* 0x51 - PgDown	*/ '3',  0,
    /* 0x52 - Insert	*/ '0',  0,
    /* 0x53 - Delete	*/ '.',  0,
    /* 0x54 - Enter		*/ 0,    0,
    /* 0x55 - ???		*/ 0,    0,
    /* 0x56 - ???		*/ 0,    0,
    /* 0x57 - F11		*/ 0,    0,
    /* 0x58 - F12		*/ 0,    0,
    /* 0x59 - ???		*/ 0,    0,
    /* 0x5A - ???		*/ 0,    0,
    /* 0x5B - ???		*/ 0,    0,
    /* 0x5C - ???		*/ 0,    0,
    /* 0x5D - ???		*/ 0,    0,
    /* 0x5E - ???		*/ 0,    0,
    /* 0x5F - ???		*/ 0,    0,
    /* 0x60 - ???		*/ 0,    0,
    /* 0x61 - ???		*/ 0,    0,
    /* 0x62 - ???		*/ 0,    0,
    /* 0x63 - ???		*/ 0,    0,
    /* 0x64 - ???		*/ 0,    0,
    /* 0x65 - ???		*/ 0,    0,
    /* 0x66 - ???		*/ 0,    0,
    /* 0x67 - ???		*/ 0,    0,
    /* 0x69 - ???		*/ 0,    0,
    /* 0x6A - ???		*/ 0,    0,
    /* 0x6B - ???		*/ 0,    0,
    /* 0x6C - ???		*/ 0,    0,
    /* 0x6D - ???		*/ 0,    0,
    /* 0x6E - ???		*/ 0,    0,
    /* 0x6F - ???		*/ 0,    0,
    /* 0x70 - ???		*/ 0,    0,
    /* 0x71 - ???		*/ 0,    0,
    /* 0x72 - ???		*/ 0,    0,
    /* 0x73 - ???		*/ 0,    0,
    /* 0x74 - ???		*/ 0,    0,
    /* 0x75 - ???		*/ 0,    0,
    /* 0x76 - ???		*/ 0,    0,
    /* 0x77 - ???		*/ 0,    0,
    /* 0x78 - ???		*/ 0,    0,
    /* 0x78 - ???		*/ 0,    0,
    /* 0x7A - ???		*/ 0,    0,
    /* 0x7B - ???		*/ 0,    0,
    /* 0x7C - ???		*/ 0,    0,
    /* 0x7D - ???		*/ 0,    0,
    /* 0x7E - ???		*/ 0,    0,
    /* 0x7F - ???		*/ 0,    0,
};

#endif
