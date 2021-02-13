#ifndef _ALPHAZ_BUGS_H_
#define _ALPHAZ_BUGS_H_

void __panic(char *fmt, char *file, int line, ...);
void __assert(char *exp, char *file, int line);

#define panic(fmt, arg...) __panic(fmt, __FILE__, __LINE__, ##arg)
#define assert(exp) \
    if (exp) ;      \
    else  __assert(#exp, __FILE__, __LINE__)

#endif
