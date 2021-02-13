#include <stdarg.h>
#include <alphaz/type.h>
#include <alphaz/stdio.h>
#include <alphaz/string.h>
#include <alphaz/ctype.h>

#include <asm/div64.h>

/* from Linux */
#define ZEROPAD 1       /* 使用0填充 */
#define SIGN	2		/* unsigned/signed long */
#define LEFT    16
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */



static char * number(char *str, char *end, unsigned long long num, int base, int size,
                        int type)
{
	char sign, tmp[66], pad;
    const char *digits;
	int i = 0;
	static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    digits = (type & LARGE) ? large_digits : small_digits;
	if (base < 2 || base > 36) return str;

    pad = (type & ZEROPAD) ? '0' : ' ';

    if (type & LEFT)
        pad = ' ';

    sign = 0;
    if(type & SIGN) {
        if ((signed long)num < 0) {
            sign = '-';
            num = -(signed long)num;
        }
    }

    if (num == 0)
		tmp[i++] = '0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num, base)];

    if (sign)
        tmp[i++] = sign;

    if (type & SPECIAL) {
        if (base == 8)
            tmp[i++] = '0';
        if (base == 16) {
            tmp[i++] = digits[33];
            tmp[i++] = '0';
        }
    }

    if (size < i)
        size = i;

    while (size > i && !(type & LEFT) && str < end) {
        *str++ = pad;
        size--;
    }

	while (str < end && i > 0 && size > 0) {
		*str++ = tmp[--i];
        size--;
    }

    while (size-- > 0 && str < end)
        *str++ = pad;

	*str = 0;
	return str;
}


/**
 * vsprintf - 格式化字符串，将其放在指定缓冲区中
 * @buf:    格式化后的字符串所放的缓冲区
 * @fmt:    要格式化成的字符串格式
 * @args:   参数列表
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
    return vsnprintf(buf, INT_MAX, fmt, args);
}


/**
 * vsnprintf - 格式化字符串，将其放在指定缓冲区中
 * @buf:    格式化后的字符串所放的缓冲区
 * @size:   缓冲区的大小
 * @fmt:    要格式化成的字符串格式
 * @args:   参数列表
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    char c, *str, *end, *s;
    int flags, base, field_width, len, qualifier;
    unsigned long long tmp;
    end = buf + size;

    for (str = buf; *fmt != 0; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }


        flags = 0;

    repeat:
        ++fmt;              /* 第一次到这里为跳过% */
        switch (*fmt) {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        field_width = 0;
        while (isdigit(*fmt)) {
            field_width = field_width * 10 + (*fmt) - '0';
            fmt++;
        }

        qualifier = -1;
        if (*fmt == 'l') {
            qualifier = 'l';
            fmt++;
            if (*fmt == 'l') {
                qualifier = 'L';
                fmt++;
            }
        }

        base = 10;

        switch (*fmt) {
        case 'c':
            c = (unsigned char)va_arg(args, int);
            if (str < end)
                *str = c;
            ++str;
            continue;

        case 's':
            s = va_arg(args, char *);
            len = strlen(s);

            field_width = field_width < len ? len : field_width;

            while (field_width > len && str < end && !(flags & LEFT)) {
                *str++ = ' ';
                field_width--;
            }

            for (; *s != 0; ++s, --field_width) {
                if (str < end)
                    *str = *s;
                str++;
            }

            while (field_width-- > 0 && str < end)
                *str++ = ' ';

            continue;

        case 'p':
            flags |= SPECIAL;
            tmp = (unsigned long)va_arg(args, void *);
            str = number(str, end, tmp, 16, field_width, flags);
            continue;

        case '%':
            if (str < end)
                *str = '%';
            ++str;
            continue;

        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            break;
        }

        if (qualifier == 'l')
            tmp = va_arg(args, unsigned long);
        else if (qualifier == 'L')
            tmp = va_arg(args, unsigned long long);
        else
            tmp = va_arg(args, unsigned int);

        str = number(str, end, tmp, base, field_width, flags);
    }
    *str = 0;
    return (str - buf);
}


/**
 * sprintf - 将格式化字符串输出到缓冲区
 * @buf: 格式化字符串输出的缓冲区
 * @fmt: 要格式化成的字符串格式
 */
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, INT_MAX, fmt, args);
	va_end(args);
	return i;
}
