# ft_philosophers

[![Philosophers CI](https://github.com/Kodoque1/ft_philosophers/actions/workflows/philo-ci.yml/badge.svg)](https://github.com/Kodoque1/ft_philosophers/actions/workflows/philo-ci.yml)

## About

42 school "Dining Philosophers" project — a classic concurrency problem solved
with POSIX threads and mutexes.

Each philosopher sits at a round table alternating between **thinking**,
**eating** and **sleeping**.  A philosopher dies if they haven't started eating
within `time_to_die` milliseconds of their last meal (or of the start of the
simulation).

## Usage

```sh
make
./philo <number_of_philosophers> <time_to_die> <time_to_eat> <time_to_sleep> [number_of_times_each_philosopher_must_eat]
```

**Example**

```sh
./philo 5 800 200 200       # 5 philos, no one should die
./philo 1 800 200 200       # 1 philo, must die (only one fork)
./philo 5 800 200 200 3     # simulation stops after each philo has eaten 3 times
```

## CI Pipeline

The repository ships with a full CI pipeline under `.github/workflows/philo-ci.yml`.

| Job | Description |
|-----|-------------|
| **Lint / Norminette** | Checks coding style with the official 42 norm checker |
| **Build (none / thread / address)** | Compiles with no sanitizer, ThreadSanitizer, and AddressSanitizer |
| **Functional tests** | Runs `philo-test.py` — validates all 5 dining-philosopher axioms and a stress loop |
| **Mock POSIX** | Builds `tests/mock_posix.so` and injects `malloc` / `pthread_create` / `pthread_mutex_lock` failures |
| **Sanitizers / Valgrind** | Runs the binary under ThreadSanitizer and Valgrind to catch races and leaks |

Logs from failing jobs are uploaded as GitHub Actions artifacts for easy inspection.

## Test Suite (`philo-test.py`)

```sh
python3 philo-test.py --binary ./philo --timeout 30 --stress-iterations 5
```

The script validates five axioms:

1. **Resources** – no philosopher holds more than 2 forks at a time
2. **Time** – log timestamps are monotonically non-decreasing
3. **Death** – a philosopher dies exactly when expected (or never, when timing allows)
4. **Progress** – philosophers keep eating; the simulation is not frozen
5. **Fairness** – every philosopher gets to eat at least once

Exit code `0` = all tests passed, `1` = at least one failure.

## Mock POSIX Library (`tests/mock_posix.c`)

Simulates syscall failures via `LD_PRELOAD` without touching the binary:

```sh
# Build the shared library
gcc -shared -fPIC -o mock_posix.so tests/mock_posix.c -ldl

# Force malloc() to fail on the very first call
TEST_MALLOC_1=1 LD_PRELOAD=./mock_posix.so ./philo 5 800 200 200

# Force pthread_create() to always fail
TEST_PTHREAD_CREATE=1 LD_PRELOAD=./mock_posix.so ./philo 5 800 200 200

# Force pthread_mutex_lock() to always fail
TEST_MUTEX_LOCK=1 LD_PRELOAD=./mock_posix.so ./philo 5 800 200 200

# ~5 % random failure on any intercepted call
TEST_RANDOM_FAIL=1 LD_PRELOAD=./mock_posix.so ./philo 5 800 200 200
```

## Project Structure

```
ft_philosophers/
├── Makefile                       # Build rules
├── main.c                         # Entry point
├── includes/
│   └── ft_philo.h                 # Shared types and prototypes
├── src/                           # Philosopher logic (threads, routines …)
├── tests/
│   └── mock_posix.c               # LD_PRELOAD failure injection library
├── philo-test.py                  # Python test harness (5 axioms + stress)
└── .github/
    └── workflows/
        └── philo-ci.yml           # Full CI pipeline
```