#ifndef _ASM_ATOMIC_H_
#define _ASM_ATOMIC_H_

typedef struct { volatile int counter; } atomic_t;

static inline void atomic_add(int i, atomic_t *atomic)
{
    asm volatile(
        "lock addl %1, %0"
        :"+m"(atomic->counter)
        :"ir"(i));
}

static inline void atomic_sub(int i, atomic_t *atomic)
{
    asm volatile(
        "lock subl %1, %0"
        :"+m"(atomic->counter)
        :"ir"(i));
}

static inline void atomic_inc(atomic_t *atomic)
{
    asm volatile(
        "lock incl %0"
        :"+m"(atomic->counter));
}

static inline void atomic_dec(atomic_t *atomic)
{
    asm volatile(
        "lock decl %0"
        :"+m"(atomic->counter));
}

/**
 * atomic_read - 读取原子锁的值
 * @v: 原子锁指针
 */
#define atomic_read(v)      ((v)->counter)

/**
 * atomic_set - 设置原子锁的值
 * @i: 要设置的整型值
 * @v: 原子锁指针
 */
#define atomic_set(i, v)    (((v)->counter) = (i))

static inline void atomic_clear_mask(int mask, atomic_t *atomic)
{
    asm volatile(
        "lock andl %1, %0"
        :"+m"(atomic->counter)
        :"ir"(~(mask)));
}

static inline void atomic_set_mask(int mask, atomic_t *atomic)
{
    asm volatile(
        "lock orl %1, %0"
        :"+m"(atomic->counter)
        :"ir"(mask));
}

#endif
