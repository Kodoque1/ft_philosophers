/*
** mock_posix.c
** ============
** Dynamic library that intercepts selected POSIX/libc calls so that the
** philosopher binary can be tested under simulated allocation and
** synchronisation failures without modifying its source code.
**
** Build:
**   gcc -shared -fPIC -o mock_posix.so tests/mock_posix.c -ldl
**
** Usage:
**   LD_PRELOAD=./mock_posix.so ./philo <args>
**
** Environment variables
** ---------------------
**   TEST_MALLOC_1=1        Force malloc()               to fail on call #1
**   TEST_MALLOC_N=<n>      Force malloc()               to fail on call #n
**   TEST_PTHREAD_CREATE=1  Force pthread_create()       to fail every time
**   TEST_MUTEX_LOCK=1      Force pthread_mutex_lock()   to fail every time
**   TEST_RANDOM_FAIL=1     Each intercepted call has a ~5 % chance to fail
**
** When a failure is injected the wrapper returns the canonical POSIX error
** value (NULL for malloc, ENOMEM / EAGAIN for pthread functions).
**
** All injected failures are logged to stderr with a distinctive prefix so
** they are easy to identify in CI logs.
*/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
** Lazy-loaded function pointers to the real libc/libpthread symbols.
** We use atomic pointers so that concurrent first-time initialisation is
** safe without a mutex (which would create a chicken-and-egg problem).
** ---------------------------------------------------------------------- */
typedef void *(*real_malloc_t)(size_t);
typedef int   (*real_pthread_create_t)(pthread_t *, const pthread_attr_t *,
                                       void *(*)(void *), void *);
typedef int   (*real_pthread_mutex_lock_t)(pthread_mutex_t *);

static _Atomic(real_malloc_t)             _real_malloc;
static _Atomic(real_pthread_create_t)     _real_pthread_create;
static _Atomic(real_pthread_mutex_lock_t) _real_pthread_mutex_lock;

/* Counters for call-number-based failures (used by TEST_MALLOC_N). */
static atomic_int _malloc_call_count = 0;

/* -------------------------------------------------------------------------
** Initialise real-function pointers via dlsym(RTLD_NEXT).
** Called lazily on the first intercepted call.
** ---------------------------------------------------------------------- */
static void init_real_malloc(void)
{
    if (!_real_malloc)
        _real_malloc = (real_malloc_t)dlsym(RTLD_NEXT, "malloc");
}

static void init_real_pthread_create(void)
{
    if (!_real_pthread_create)
        _real_pthread_create =
            (real_pthread_create_t)dlsym(RTLD_NEXT, "pthread_create");
}

static void init_real_pthread_mutex_lock(void)
{
    if (!_real_pthread_mutex_lock)
        _real_pthread_mutex_lock =
            (real_pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
}

/* -------------------------------------------------------------------------
** Lightweight helpers
** ---------------------------------------------------------------------- */

/* Return 1 if the named environment variable is set to "1", 0 otherwise. */
static int env_flag(const char *name)
{
    const char *v = getenv(name);
    return v && strcmp(v, "1") == 0;
}

/* Return the integer value of an environment variable, or -1 if unset. */
static int env_int(const char *name)
{
    const char *v = getenv(name);
    return v ? atoi(v) : -1;
}

/* Pseudo-random failure with a probability of roughly 1/20 (~5 %).
** Uses a simple LCG seeded from the pointer address to avoid depending on
** rand_r state shared across threads. */
static int random_fail(void)
{
    if (!env_flag("TEST_RANDOM_FAIL"))
        return 0;
    /* Fast, thread-safe, good-enough LCG */
    static atomic_uint _seed = 0;
    unsigned int s = atomic_fetch_add(&_seed, 1u) * 1664525u + 1013904223u;
    return (s % 20) == 0; /* ~5 % */
}

/* -------------------------------------------------------------------------
** malloc interception
** ---------------------------------------------------------------------- */
void *malloc(size_t size)
{
    init_real_malloc();

    int call_n = atomic_fetch_add(&_malloc_call_count, 1) + 1;

    /* Fail on a specific call number (TEST_MALLOC_N) */
    int target = env_int("TEST_MALLOC_N");
    if (target > 0 && call_n == target)
    {
        fprintf(stderr,
            "[mock_posix] malloc() call #%d INJECTED FAILURE (TEST_MALLOC_N=%d)\n",
            call_n, target);
        errno = ENOMEM;
        return NULL;
    }

    /* Fail on the very first call (shorthand: TEST_MALLOC_1=1) */
    if (env_flag("TEST_MALLOC_1") && call_n == 1)
    {
        fprintf(stderr,
            "[mock_posix] malloc() call #1 INJECTED FAILURE (TEST_MALLOC_1)\n");
        errno = ENOMEM;
        return NULL;
    }

    /* Random failure */
    if (random_fail())
    {
        fprintf(stderr,
            "[mock_posix] malloc() call #%d INJECTED RANDOM FAILURE\n", call_n);
        errno = ENOMEM;
        return NULL;
    }

    return _real_malloc(size);
}

/* -------------------------------------------------------------------------
** pthread_create interception
** ---------------------------------------------------------------------- */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg)
{
    init_real_pthread_create();

    if (env_flag("TEST_PTHREAD_CREATE"))
    {
        fprintf(stderr,
            "[mock_posix] pthread_create() INJECTED FAILURE (TEST_PTHREAD_CREATE)\n");
        return EAGAIN;
    }

    if (random_fail())
    {
        fprintf(stderr,
            "[mock_posix] pthread_create() INJECTED RANDOM FAILURE\n");
        return EAGAIN;
    }

    return _real_pthread_create(thread, attr, start_routine, arg);
}

/* -------------------------------------------------------------------------
** pthread_mutex_lock interception
** ---------------------------------------------------------------------- */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    init_real_pthread_mutex_lock();

    if (env_flag("TEST_MUTEX_LOCK"))
    {
        fprintf(stderr,
            "[mock_posix] pthread_mutex_lock() INJECTED FAILURE (TEST_MUTEX_LOCK)\n");
        return EDEADLK;
    }

    if (random_fail())
    {
        fprintf(stderr,
            "[mock_posix] pthread_mutex_lock() INJECTED RANDOM FAILURE\n");
        return EDEADLK;
    }

    return _real_pthread_mutex_lock(mutex);
}
