#ifndef _TYPE_H_
#define _TYPE_H_

typedef char               int8;
typedef short              int16;
typedef int                int32;
typedef long long          int64;
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef unsigned long long u64;

typedef unsigned long size_t;
typedef long long     loff_t;
typedef int           pid_t;

typedef long ssize_t;

#define NULL    0
#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)

#define offsetof(type, member) ((size_t) & ((type *)0)->member)

#define upper_div(dividend, divisor) ((dividend) / (divisor) + ((dividend) % (divisor) > 0))
#define lower_div(dividend, divisor) ((dividend) / (divisor))

#endif
