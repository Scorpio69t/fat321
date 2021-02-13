#include <alphaz/keyboard.h>
#include <alphaz/kernel.h>
#include <alphaz/type.h>
#include <alphaz/bugs.h>
#include <alphaz/wait.h>
#include <alphaz/slab.h>
#include <alphaz/malloc.h>
#include <alphaz/spinlock.h>

#include <asm/irq.h>
#include <asm/io.h>

struct file *stdin;

#define KEYBOARD_BUF_SIZE   128
#define NR_SCAN_CODES       0x80
#define MAP_COLS            2

struct {
    char buf[KEYBOARD_BUF_SIZE];
    int count;               /* 缓冲区扫描码数 */
    int head, tail;
} kb_buf;

struct {
    unsigned char shift_l;
    unsigned char shift_r;
    unsigned char ctrl_l;
    unsigned char ctrl_r;
    unsigned char alt_l;
    unsigned char alt_r;
} kbf;  /* keyboard flags */

wait_queue_head_t kb_wait_head;

#define FLAG_BREAK      0x80

/* Keymap for US MF-2 keyboard. */
unsigned char keymap[NR_SCAN_CODES * MAP_COLS] = {
/* scan-code			!Shift		Shift */
/* 0x00 - none		*/	0,		    0,
/* 0x01 - ESC		*/	0,		    0,
/* 0x02 - '1'		*/	'1',		'!',
/* 0x03 - '2'		*/	'2',		'@',
/* 0x04 - '3'		*/	'3',		'#',
/* 0x05 - '4'		*/	'4',		'$',
/* 0x06 - '5'		*/	'5',		'%',
/* 0x07 - '6'		*/	'6',		'^',
/* 0x08 - '7'		*/	'7',		'&',
/* 0x09 - '8'		*/	'8',		'*',
/* 0x0A - '9'		*/	'9',		'(',
/* 0x0B - '0'		*/	'0',		')',
/* 0x0C - '-'		*/	'-',		'_',
/* 0x0D - '='		*/	'=',		'+',
/* 0x0E - BS		*/	'\b',	    0,
/* 0x0F - TAB		*/	'\t',		0,
/* 0x10 - 'q'		*/	'q',		'Q',
/* 0x11 - 'w'		*/	'w',		'W',
/* 0x12 - 'e'		*/	'e',		'E',
/* 0x13 - 'r'		*/	'r',		'R',
/* 0x14 - 't'		*/	't',		'T',
/* 0x15 - 'y'		*/	'y',		'Y',
/* 0x16 - 'u'		*/	'u',		'U',
/* 0x17 - 'i'		*/	'i',		'I',
/* 0x18 - 'o'		*/	'o',		'O',
/* 0x19 - 'p'		*/	'p',		'P',
/* 0x1A - '['		*/	'[',		'{',
/* 0x1B - ']'		*/	']',		'}',
/* 0x1C - CR/LF		*/	'\n',		0,
/* 0x1D - l. Ctrl	*/	0x1d,		0x1d,
/* 0x1E - 'a'		*/	'a',		'A',
/* 0x1F - 's'		*/	's',		'S',
/* 0x20 - 'd'		*/	'd',		'D',
/* 0x21 - 'f'		*/	'f',		'F',
/* 0x22 - 'g'		*/	'g',		'G',
/* 0x23 - 'h'		*/	'h',		'H',
/* 0x24 - 'j'		*/	'j',		'J',
/* 0x25 - 'k'		*/	'k',		'K',
/* 0x26 - 'l'		*/	'l',		'L',
/* 0x27 - ';'		*/	';',		':',
/* 0x28 - '\''		*/	'\'',		'"',
/* 0x29 - '`'		*/	'`',		'~',
/* 0x2A - l. SHIFT	*/	0x2a,	    0x2a,
/* 0x2B - '\'		*/	'\\',		'|',
/* 0x2C - 'z'		*/	'z',		'Z',
/* 0x2D - 'x'		*/	'x',		'X',
/* 0x2E - 'c'		*/	'c',		'C',
/* 0x2F - 'v'		*/	'v',		'V',
/* 0x30 - 'b'		*/	'b',		'B',
/* 0x31 - 'n'		*/	'n',		'N',
/* 0x32 - 'm'		*/	'm',		'M',
/* 0x33 - ','		*/	',',		'<',
/* 0x34 - '.'		*/	'.',		'>',
/* 0x35 - '/'		*/	'/',		'?',
/* 0x36 - r. SHIFT	*/	0x36,	    0x36,
/* 0x37 - '*'		*/	'*',		'*',
/* 0x38 - ALT		*/	0x38,		0x38,
/* 0x39 - ' '		*/	' ',		' ',
/* 0x3A - CapsLock	*/	0,	        0,
/* 0x3B - F1		*/	0,		    0,
/* 0x3C - F2		*/	0,		    0,
/* 0x3D - F3		*/	0,		    0,
/* 0x3E - F4		*/	0,		    0,
/* 0x3F - F5		*/	0,		    0,
/* 0x40 - F6		*/	0,		    0,
/* 0x41 - F7		*/	0,		    0,
/* 0x42 - F8		*/	0,		    0,
/* 0x43 - F9		*/	0,		    0,
/* 0x44 - F10		*/	0,		    0,
/* 0x45 - NumLock	*/	0,	        0,
/* 0x46 - ScrLock	*/	0,          0,
/* 0x47 - Home		*/	'7',        0,
/* 0x48 - CurUp		*/	'8',        0,
/* 0x49 - PgUp		*/	'9',        0,
/* 0x4A - '-'		*/	'-',        0,
/* 0x4B - Left		*/	'4',        0,
/* 0x4C - MID		*/	'5',        0,
/* 0x4D - Right		*/	'6',        0,
/* 0x4E - '+'		*/	'+',        0,
/* 0x4F - End		*/	'1',        0,
/* 0x50 - Down		*/	'2',        0,
/* 0x51 - PgDown	*/	'3',        0,
/* 0x52 - Insert	*/	'0',        0,
/* 0x53 - Delete	*/	'.',        0,
/* 0x54 - Enter		*/	0,	    	0,
/* 0x55 - ???		*/	0,	    	0,
/* 0x56 - ???		*/	0,		    0,
/* 0x57 - F11		*/	0,		    0,
/* 0x58 - F12		*/	0,  		0,
/* 0x59 - ???		*/	0,	    	0,
/* 0x5A - ???		*/	0,	    	0,
/* 0x5B - ???		*/	0,	    	0,
/* 0x5C - ???		*/	0,	    	0,
/* 0x5D - ???		*/	0,	    	0,
/* 0x5E - ???		*/	0,	    	0,
/* 0x5F - ???		*/	0,	    	0,
/* 0x60 - ???		*/	0,	    	0,
/* 0x61 - ???		*/	0,	    	0,
/* 0x62 - ???		*/	0,	    	0,
/* 0x63 - ???		*/	0,	    	0,
/* 0x64 - ???		*/	0,	    	0,
/* 0x65 - ???		*/	0,	    	0,
/* 0x66 - ???		*/	0,	    	0,
/* 0x67 - ???		*/	0,	    	0,
/* 0x69 - ???		*/	0,	    	0,
/* 0x6A - ???		*/	0,	    	0,
/* 0x6B - ???		*/	0,	    	0,
/* 0x6C - ???		*/	0,	    	0,
/* 0x6D - ???		*/	0,	    	0,
/* 0x6E - ???		*/	0,	    	0,
/* 0x6F - ???		*/	0,	    	0,
/* 0x70 - ???		*/	0,	    	0,
/* 0x71 - ???		*/	0,	    	0,
/* 0x72 - ???		*/	0,	    	0,
/* 0x73 - ???		*/	0,	    	0,
/* 0x74 - ???		*/	0,	    	0,
/* 0x75 - ???		*/	0,	    	0,
/* 0x76 - ???		*/	0,	    	0,
/* 0x77 - ???		*/	0,	    	0,
/* 0x78 - ???		*/	0,	    	0,
/* 0x78 - ???		*/	0,	    	0,
/* 0x7A - ???		*/	0,	    	0,
/* 0x7B - ???		*/	0,	    	0,
/* 0x7C - ???		*/	0,	    	0,
/* 0x7D - ???		*/	0,	    	0,
/* 0x7E - ???		*/	0,	    	0,
/* 0x7F - ???		*/	0,	    	0,
};


static inline void init_buf(void)
{
    kb_buf.count = kb_buf.head = kb_buf.tail = 0;
    memset(kb_buf.buf, 0, KEYBOARD_BUF_SIZE);
}

static void keyboard_handle(struct pt_regs *regs, unsigned no)
{
    if (kb_buf.count < KEYBOARD_BUF_SIZE) {
        kb_buf.buf[kb_buf.head] = read_scancode();
        kb_buf.head = (kb_buf.head + 1) % KEYBOARD_BUF_SIZE;
        kb_buf.count++;
        wake_up(&kb_wait_head);
        current->flags |= NEED_SCHEDULE;
    }
}


static char get_scancode(void)
{
    unsigned char code;

    if (!kb_buf.count)
        sleep_on(&kb_wait_head);
    code = kb_buf.buf[kb_buf.tail];
    kb_buf.tail = (kb_buf.tail + 1) % KEYBOARD_BUF_SIZE;
    kb_buf.count--;
    return code;
}

/**
 * 将扫描码解析成ascii码或控制命令, 返回ascii码
 */
static char annlysis_scancode(void)
{
    unsigned char code;
    int i, key, make;

loop:
    key = make = 0;
    code = get_scancode();

    if (code == 0xe1) {
        /* Pause被按下，读出后续的五个扫描码，不做任何操作 */
        for (i = 0; i < 5; i++)
            get_scancode();
    } else if (code == 0xe0) {
        code = get_scancode();
        switch (code) {
        case 0x1d:  /* 按下右ctrl */
            kbf.ctrl_r = 1;
            break;
        case 0x9d:  /* 松开右ctrl */
            kbf.ctrl_r = 0;
            break;
        case 0x38:  /* 按下右alt */
            kbf.alt_r = 1;
            break;
        case 0xb8:  /* 松开右alt */
            kbf.alt_r = 0;
            break;
        default:
            break;
        }
    } else {
        make = code & FLAG_BREAK ? 0 : 1;

        switch (code & 0x7f) {
        case 0x2a:
            kbf.shift_l = make;
            break;
        case 0x36:
            kbf.shift_r = make;
            break;
        case 0x1d:
            kbf.ctrl_l = make;
            break;
        case 0x38:
            kbf.alt_l = make;
            break;
        default:
            if (make)
                key = keymap[(code & 0x7f) * MAP_COLS + (kbf.shift_l | kbf.shift_r)];
            break;
        }
    }
    if (!key)
        goto loop;
    return key;
}


static ssize_t kb_read(struct file *filp, char *buf, size_t n, loff_t pos)
{
    int i;
    for (i = 0; i < n; i++)
        buf[i] = annlysis_scancode();
    return n;
}

static struct file_operations keyboard_operations = {
    .read = kb_read,
};

/**
 * 键盘处理初始化
 */
void keyboard_init(void)
{
    init_buf();
    init_wait_queue_head(&kb_wait_head);

    memset(&kbf, 0, sizeof(kbf));

    stdin = (struct file *)kmalloc(sizeof(struct file), 0);
    atomic_set(1, &stdin->f_count);
    stdin->f_dentry = NULL;
    stdin->f_flags = 0;
    spin_init(&stdin->f_lock);
    stdin->f_mode = 0;
    stdin->f_op = &keyboard_operations;
    stdin->f_pos = 0;

    __keyboard_init();

    if (register_irq(0x21, keyboard_handle))
        panic("keyboard register error\n");
}
