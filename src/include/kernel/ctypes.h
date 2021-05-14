#ifndef _KERNEL_CTYPE_H_
#define _KERNEL_CTYPE_H_

static inline int isalpha(int c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

static inline int isblank(int c)
{
    if (c == ' ' || c == '\t')
        return 1;
    return 0;
}

static inline int isspace(int c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
        return 1;
    return 0;
}

static inline int isdigit(int c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

static inline int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

static inline int isprint(int c)
{
    if (c >= 0x20 && c <= 0x7e)
        return 1;
    return 0;
}

static inline int iscntrl(int c)
{
    if ((c >= 0x00 && c <= 0x1f) || c == 0x7f)
        return 1;
    return 0;
}

static inline int ispunct(int c)
{
    if (c >= 0x21 && c <= 0x2f)
        return 1;
    if (c >= 0x3a && c <= 0x40)
        return 1;
    if (c >= 0x5b && c <= 0x60)
        return 1;
    if (c >= 0x7b && c <= 0x7e)
        return 1;
    return 0;
}

static inline int islower(int c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    return 0;
}

static inline int isupper(int c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

static inline int isxdigit(int c)
{
    if (c >= '0' && c <= '9')
        return 1;
    if (c >= 'A' && c <= 'F')
        return 1;
    if (c >= 'a' && c <= 'f')
        return 1;
    return 0;
}

static inline int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    return c;
}

static inline int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');
    return c;
}

#endif
