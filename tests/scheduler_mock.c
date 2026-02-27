/*
** scheduler_mock.c
** ================
** LD_PRELOAD wrapper library that intercepts pthread_create and
** pthread_mutex_lock and injects small artificial delays before each call.
**
** The goal is NOT to cause failures (see tests/mock_posix.c for that) but
** to push threads into rare orderings, dramatically increasing the chance of
** surfacing data races and deadlocks that would otherwise require millions of
** normal runs to trigger.
**
** Combined with ThreadSanitizer (TSAN) this provides two independent layers
** of concurrency-bug detection:
**   1. scheduler_mock.so  – triggers rare interleavings via timing nudges
**   2. ThreadSanitizer    – catches actual unsynchronised memory accesses
**
** Build:
**   gcc -shared -fPIC -o libscheduler_mock.so tests/scheduler_mock.c -ldl
**
** Usage (via a thin wrapper script so delays stay inside philo, not Python):
**   LD_PRELOAD=./libscheduler_mock.so ./philo 5 800 200 200
**
** Environment variables
** ---------------------
**   SCHED_MOCK_MODE=random       Random delay 0–SCHED_MOCK_MAX_US µs per call
**                                (default).
**   SCHED_MOCK_MODE=sequential   Rotating delay that cycles through 10 evenly-
**                                spaced values so every timing position is
**                                exercised systematically across calls.
**   SCHED_MOCK_MAX_US=<n>        Maximum delay in microseconds (default: 500).
**                                500 µs is safe even for time_to_die = 310 ms.
**   SCHED_MOCK_SEED=<n>          PRNG seed for reproducible random runs
**                                (default: 42).
**   SCHED_MOCK_VERBOSE=1         Print each injected delay to stderr.
*/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
** Real function pointers resolved lazily via dlsym(RTLD_NEXT).
** Atomic storage avoids a mutex bootstrapping problem (we would need a mutex
** to protect the pointer while overriding pthread_mutex_lock itself).
** ---------------------------------------------------------------------- */
typedef int (*real_pthread_create_t)(pthread_t *, const pthread_attr_t *,
                                     void *(*)(void *), void *);
typedef int (*real_pthread_mutex_lock_t)(pthread_mutex_t *);

static _Atomic(real_pthread_create_t)     _real_pthread_create;
static _Atomic(real_pthread_mutex_lock_t) _real_pthread_mutex_lock;

/* Sequential counter used in "sequential" mode. */
static atomic_uint _seq_counter = 0;

/* -------------------------------------------------------------------------
** Lazy initialisation helpers
**
** We use a compare-exchange to make the first-time initialisation
** thread-safe without a mutex (which would create a bootstrapping problem
** since we are overriding pthread_mutex_lock itself).  If two threads race
** on the very first call, both will resolve the same symbol via dlsym and
** the CAS ensures only one value is stored; the other is discarded harmlessly.
** ---------------------------------------------------------------------- */
static void init_real_pthread_create(void)
{
	if (!atomic_load(&_real_pthread_create))
	{
		real_pthread_create_t p =
			(real_pthread_create_t)dlsym(RTLD_NEXT, "pthread_create");
		real_pthread_create_t expected = NULL;
		atomic_compare_exchange_strong(&_real_pthread_create, &expected, p);
	}
}

static void init_real_pthread_mutex_lock(void)
{
	if (!atomic_load(&_real_pthread_mutex_lock))
	{
		real_pthread_mutex_lock_t p =
			(real_pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
		real_pthread_mutex_lock_t expected = NULL;
		atomic_compare_exchange_strong(&_real_pthread_mutex_lock, &expected, p);
	}
}

/* -------------------------------------------------------------------------
** Delay computation
** ---------------------------------------------------------------------- */

/* Return the maximum delay (µs) from SCHED_MOCK_MAX_US (default 500). */
static unsigned int get_max_us(void)
{
	const char	*v = getenv("SCHED_MOCK_MAX_US");

	if (v && *v)
	{
		int n = ft_atoi(v);
		if (n > 0)
			return ((unsigned int)n);
	}
	return (500u);
}

/*
** Compute the next delay value (µs) for this interception.
**
** random mode (default):
**   Uses a lock-free LCG (linear congruential generator) seeded from
**   SCHED_MOCK_SEED so that runs are reproducible when the same seed is
**   provided.  The state advances with each call so concurrent threads get
**   different delay values without synchronisation overhead.
**
** sequential mode:
**   Cycles through 10 evenly-spaced values (0, step, 2*step … 9*step) so
**   every "position" in the schedule is explored systematically.
*/
static unsigned int next_delay_us(void)
{
	unsigned int	max_us = get_max_us();
	const char		*mode = getenv("SCHED_MOCK_MODE");

	if (mode && strcmp(mode, "sequential") == 0)
	{
		/* 10 positions: 0, step, 2*step, … 9*step */
		unsigned int step = (max_us > 9u) ? max_us / 10u : 1u;
		unsigned int pos  = atomic_fetch_add(&_seq_counter, 1u) % 10u;
		return (pos * step);
	}

	/* Default: random mode — lock-free LCG using compare-exchange loop.
	** The state advances via state = state * A + C so consecutive calls
	** each get the next value in a proper LCG sequence. */
	{
		static atomic_uint _lcg_state = 0;
		/* One-time seed initialisation via compare-exchange. */
		static atomic_int _seeded = 0;
		if (!_seeded)
		{
			int expected = 0;
			if (atomic_compare_exchange_strong(&_seeded, &expected, 1))
			{
				const char   *s    = getenv("SCHED_MOCK_SEED");
				unsigned int  seed = s ? (unsigned int)ft_atoi(s) : 42u;
				atomic_store(&_lcg_state, seed);
			}
		}
		/* CAS loop: atomically advance the LCG state and capture old value. */
		unsigned int old_s, new_s;
		do {
			old_s = atomic_load(&_lcg_state);
			new_s = old_s * 1664525u + 1013904223u;
		} while (!atomic_compare_exchange_weak(&_lcg_state, &old_s, new_s));
		return (old_s % (max_us + 1u));
	}
}

/* Inject a small delay before an intercepted call. */
static void inject_delay(const char *sym)
{
	unsigned int delay_us = next_delay_us();

	if (delay_us == 0)
		return ;
	if (getenv("SCHED_MOCK_VERBOSE"))
		fprintf(stderr, "[scheduler_mock] %s: +%u µs\n", sym, delay_us);
	usleep(delay_us);
}

/* -------------------------------------------------------------------------
** pthread_create interception
** Delays thread launch to explore different startup orderings.
** ---------------------------------------------------------------------- */
int	pthread_create(pthread_t *thread, const pthread_attr_t *attr,
					void *(*start_routine)(void *), void *arg)
{
	init_real_pthread_create();
	inject_delay("pthread_create");
	return (_real_pthread_create(thread, attr, start_routine, arg));
}

/* -------------------------------------------------------------------------
** pthread_mutex_lock interception
** Delays each lock attempt so threads arrive at critical-section entries in
** different orders, increasing the chance of exposing TOCTOU races.
** ---------------------------------------------------------------------- */
int	pthread_mutex_lock(pthread_mutex_t *mutex)
{
	init_real_pthread_mutex_lock();
	inject_delay("pthread_mutex_lock");
	return (_real_pthread_mutex_lock(mutex));
}
