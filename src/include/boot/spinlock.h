#ifndef _ASM_SPINLOCK_H_
#define _ASM_SPINLOCK_H_

/* 值为1为加锁，其他值表示加锁 */
typedef struct { volatile unsigned int lock; } spinlock_t;

static inline int __spin_is_locked(spinlock_t *lock)
{
    return *(volatile signed char *)(&(lock)->lock) <= 0;
}

static inline void __spin_lock(spinlock_t *lock)
{
    asm volatile(
        "\n1:\t lock decl %0\n\t"
        "jns  3f\n\t"
        "2:\t pause\n\t"
        "cmpb $0, %0\n\t"
        "jle 2b\n\t"
        "jmp 1b\n\t"
        "3:\t"
        :"+m"(lock->lock)::"memory");
}

static inline void __spin_unlock(spinlock_t *lock)
{
    asm volatile("movb $1, %0":"+m"(lock->lock)::"memory");
}

static inline int __spin_try_lock(spinlock_t *lock)
{
    char oldval;
    asm volatile(
        "xchgb %b0, %1\n\t"
        :"=q"(oldval), "+m"(lock->lock)
        :"0"(0)
        :"memory");
    return oldval > 0;
}

#endif
