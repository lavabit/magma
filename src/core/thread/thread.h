
/**
 * @file /magma/core/thread/thread.h
 *
 * @brief	Interface for providing pthread functionality.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_THREAD_H
#define MAGMA_CORE_THREAD_H

/// thread.c
pthread_t *  thread_alloc(void *function, void *data);
int_t        thread_cancel(pthread_t thread);
void         thread_cancel_disable(void);
void         thread_cancel_enable(void);
pthread_t    thread_get_thread_id(void);
int_t        thread_join(pthread_t thread);
int_t        thread_launch(pthread_t *thread, void *function, void *data);
int_t        thread_result(pthread_t thread, void **result);
int_t        thread_signal(pthread_t thread, int_t signal);

/// rwlock.c
int   rwlock_attr_destroy(pthread_rwlockattr_t *attr);
int   rwlock_attr_getkind(pthread_rwlockattr_t *attr, int *pref);
int   rwlock_attr_init(pthread_rwlockattr_t *attr);
int   rwlock_attr_setkind(pthread_rwlockattr_t *attr, int pref);
int   rwlock_destroy(pthread_rwlock_t *lock);
int   rwlock_init(pthread_rwlock_t *lock, pthread_rwlockattr_t * attr);
int   rwlock_lock_read(pthread_rwlock_t *lock);
int   rwlock_lock_write(pthread_rwlock_t *lock);
int   rwlock_unlock(pthread_rwlock_t *lock);

/// keys.c
void *  tkey_get(pthread_key_t key);
int     tkey_init(pthread_key_t *key, void(*destructor)(void*));
int     tkey_set(pthread_key_t key, void *value);

/// mutex.c
int   mutex_destroy(pthread_mutex_t *lock);
int   mutex_init(pthread_mutex_t *lock, pthread_mutexattr_t *attr);
int   mutex_lock(pthread_mutex_t *lock);
int   mutex_unlock(pthread_mutex_t *lock);

#endif
