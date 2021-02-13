/*
 * 自旋锁相关接口定义
 *
 * 当前的接口只是形式定义，仅有部分功能，并且当前的内核还并未实现多处理器。之所以提前定义相关
 * 的锁结构是为了让程序按照多处理器模式进行开发，方便以后向多处理器方向扩充
 */

#ifndef _ALPHAZ_SPINLOCK_H_
#define _ALPHAZ_SPINLOCK_H_

#include <asm/spinlock.h>

static inline void spin_init(spinlock_t *lock)
{
    lock->lock = 1;
}

static inline int spin_is_locked(spinlock_t *lock)
{
    return __spin_is_locked(lock);
}

static inline void spin_lock(spinlock_t *lock)
{
    __spin_lock(lock);
}

static inline void spin_unlock(spinlock_t *lock)
{
    __spin_unlock(lock);
}

static inline int spin_try_lock(spinlock_t *lock)
{
    return __spin_try_lock(lock);
}

#endif
