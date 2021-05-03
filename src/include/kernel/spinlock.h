#ifndef _KERNEL_SPINLOCK_H_
#define _KERNEL_SPINLOCK_H_

#include <boot/spinlock.h>

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
