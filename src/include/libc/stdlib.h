#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <ctype.h>

void exit(int);

static inline int atoi(const char *nptr)
{
    int n, minus;
    char *ptr;

    ptr = (char *)nptr;
    minus = 1;
    while (isspace(*ptr)) ptr++;
    if (*ptr == '-') {
        minus = -1;
        ptr++;
    }
    for (n = 0; *ptr != 0 && *ptr != '.'; ptr++) {
        if (!isdigit(*ptr))
            return 0;
        n = n * 10 + *ptr - '0';
    }
    return minus * n;
}
#endif
