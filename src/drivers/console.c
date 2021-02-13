#include <alphaz/type.h>
#include <alphaz/wait.h>
#include <alphaz/stdio.h>
#include <alphaz/string.h>
#include <alphaz/fs.h>
#include <alphaz/malloc.h>
#include <alphaz/console.h>
#include <alphaz/bugs.h>
#include <alphaz/slab.h>
#include <alphaz/fontdata.h>
#include <alphaz/config.h>

#include <asm/console.h>
#include <asm/div64.h>
#include <asm/io.h>

struct console_desc console;

#ifndef __VBE
#define ROW     25
#define COL     80
#else
#define ROW     (console.height / font_8x16.height)
#define COL     (console.width / font_8x16.width)
#endif

struct file *stdout;
struct file *stderr;

/**
 * 向控制台写字符串
 * @buf: 字符串缓冲区
 * @n: 字符串长度
 * @type: 字符串颜色属性
 */
ssize_t console_write(const char *buf, size_t n, unsigned char type)
{
    int i, cur;

    cur = get_cursor();
    for (i = 0; i < n; i++) {
        switch (buf[i]) {
        case '\n':
            cur = (cur / COL + 1) * COL;
            break;
        case '\t':
            cur = cur + 4;
            break;
        case '\b':
            cur = cur - 1;
            write_char(' ', type, (unsigned short)cur);
            break;
        default:
            write_char(buf[i], type, (unsigned short)cur);
            cur++;
        }
        if (cur >= ROW * COL) {
            cur = (ROW - 1) * COL;
            console_curl(1);
        }
        set_cursor(cur);
    }
    return n;
}

static loff_t lseek(struct file *filp, loff_t pos, int mode)
{
    int cur;

    cur = get_cursor();
    switch (mode) {
    case SEEK_SET:
        if (pos >= 0 && pos < ROW * COL)
            cur = pos;
        else
            return -1;
        break;
    case SEEK_CUR:
        if (cur + pos >= 0 && cur < ROW * COL)
            cur = cur + pos;
        else
            return -1;
        break;
    case SEEK_END:
        return -1;
    }
    set_cursor(cur);
    return cur;
}

static ssize_t write(struct file *filp, const char *buf, size_t n, loff_t pos)
{
    return console_write(buf, n, 0x0f);
}

static struct file_operations console_operations = {
    .lseek = lseek,
    .read = NULL,
    .write = write,
    .open = NULL,
    .release = NULL,
};

static struct file * make_dev_file(struct file_operations *fo)
{
    struct file *filp;

    filp = (struct file *)kmalloc(sizeof(struct file), 0);
    assert(filp != NULL);
    filp->f_dentry = NULL;
    filp->f_op = fo;
    spin_init(&filp->f_lock);
    atomic_set(1, &filp->f_count);
    filp->f_flags = 0;
    filp->f_mode = 0;
    filp->f_pos = 0;
    return filp;
}

void clear_screen(void)
{
    int i;

    for (i = 0; i < ROW * COL; i++) {
        write_char(' ', 0x0f, i);
    }
    set_cursor(0);
}

void console_init(void)
{
    stdout = make_dev_file(&console_operations);
    stderr = make_dev_file(&console_operations);
}
