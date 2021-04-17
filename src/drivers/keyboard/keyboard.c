#include "keyboard.h"

#include <stdio.h>
#include <string.h>
#include <sys/io.h>
#include <sys/ipc.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define KEYBOARD_BUF_SIZE 128
struct {
    char buf[KEYBOARD_BUF_SIZE];
    int  count; /* 字符数量 */
    int  head, tail;
} kb_buf;

struct {
    unsigned char shift_l;
    unsigned char shift_r;
    unsigned char ctrl_l;
    unsigned char ctrl_r;
    unsigned char alt_l;
    unsigned char alt_r;
} kbf; /* keyboard flags */

static inline void init_buf(void)
{
    kb_buf.count = kb_buf.head = kb_buf.tail = 0;
    memset(kb_buf.buf, 0, KEYBOARD_BUF_SIZE);
}

static void put_char(char c)
{
    if (kb_buf.count >= KEYBOARD_BUF_SIZE) /* discard it */
        return;
    kb_buf.buf[kb_buf.tail] = c;
    kb_buf.tail = (kb_buf.tail + 1) % KEYBOARD_BUF_SIZE;
    kb_buf.count++;
    // debug("%c", c);
}

static int read_char(char *buf, size_t size)
{
    int sz = 0;
    while (kb_buf.head != kb_buf.tail && sz <= size) {
        buf[sz++] = kb_buf.buf[kb_buf.head];
        kb_buf.head = (kb_buf.head + 1) % KEYBOARD_BUF_SIZE;
        kb_buf.count--;
    }
    return sz;
}

/**
 * 将扫描码解析成ascii码或控制命令, 返回ascii码
 */
static void annlysis_scancode(void)
{
    static char   pre_code = 0;
    static int    skip = 0;
    unsigned char code;
    int           key, make;

    key = make = 0;
    code = inb(0x60);

    if (code == 0xe1) {
        pre_code = 0xe1;
        skip = 5;
        return;
    }
    if (code == 0xe0) {
        pre_code = 0xe1;
        return;
    }

    if (pre_code == 0xe1) {
        skip--;
        if (!skip)
            pre_code = 0;
        return;
    } else if (pre_code == 0xe0) {
        switch (code) {
        case 0x1d: /* 按下右ctrl */
            kbf.ctrl_r = 1;
            break;
        case 0x9d: /* 松开右ctrl */
            kbf.ctrl_r = 0;
            break;
        case 0x38: /* 按下右alt */
            kbf.alt_r = 1;
            break;
        case 0xb8: /* 松开右alt */
            kbf.alt_r = 0;
            break;
        default:
            break;
        }
        pre_code = 0;
        return;
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
        return;
    put_char(key);
}

int main(void)
{
    while (inb(0x64) & 0x02) nop(); /* 8042缓冲区满则循环 */
    outb(0x64, 0x60);
    while (inb(0x64) & 0x02) nop();
    outb(0x60, 0x65); /* 使用第一套扫描码，只使能键盘 */

    if (register_irq(0x21) != 0) {
        debug("keyboard register error\n");
        while (1) nop();
    }

    init_buf();
    memset(&kbf, 0, sizeof(kbf));

    while (1) {
        message m;
        if (_recv(IPC_ALL, &m) != 0) {
            debug("keyboard message recv error\n");
            continue;
        }
        if (m.type == MSG_INTR) {
            annlysis_scancode();
            continue;
        }

        if (m.type == MSG_FSREAD) {
            message tmp;
            while (!kb_buf.count) {
                if (_recv(IPC_INTR, &tmp) != 0) {
                    debug("keyboard intr message recv error\n");
                    goto failed;
                }
                annlysis_scancode();
            }
            m.retval = read_char(m.m_fsread.buf, m.m_fsread.size);
            if (_send(m.src, &m) != 0) {
                debug("kb send msg error\n");
            }
            continue;
        }
        debug("kb: unsupport message\n");
    failed:
        m.retval = -1;
        _send(m.src, &m);
    }
    return 0;
}
