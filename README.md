# ft_philosophers

[![Philosophers CI](https://github.com/Kodoque1/ft_philosophers/actions/workflows/philo-ci.yml/badge.svg)](https://github.com/Kodoque1/ft_philosophers/actions/workflows/philo-ci.yml)
[![Norminette](https://github.com/Kodoque1/ft_philosophers/actions/workflows/norminette.yml/badge.svg)](https://github.com/Kodoque1/ft_philosophers/actions/workflows/norminette.yml)
[![Sanitizers](https://github.com/Kodoque1/ft_philosophers/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/Kodoque1/ft_philosophers/actions/workflows/sanitizers.yml)
[![Static Analysis](https://github.com/Kodoque1/ft_philosophers/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/Kodoque1/ft_philosophers/actions/workflows/static-analysis.yml)

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

The repository ships with four focused CI workflows, each with its own badge.

### `philo-ci.yml` — Core pipeline (build + functional tests)

| Job | Description |
|-----|-------------|
| **Build (none / thread / address)** | Compiles with no sanitizer, ThreadSanitizer, and AddressSanitizer |
| **Functional tests** | Runs `philo-test.py` — validates all 5 dining-philosopher axioms and a stress loop |
| **Mock POSIX** | Builds `tests/mock_posix.so` and injects `malloc` / `pthread_create` / `pthread_mutex_lock` failures |
| **Scheduler Mock (stress)** | Builds `tests/scheduler_mock.so`, runs `philo-test.py` with artificial delays to expose rare thread interleavings |

### `norminette.yml` — Style check

| Job | Description |
|-----|-------------|
| **Norminette** | Checks coding style with the official 42 norm checker |

### `sanitizers.yml` — Volatility / memory-safety

| Job | Description |
|-----|-------------|
| **ThreadSanitizer** | Detects data races at runtime |
| **Valgrind** | Checks for memory errors and leaks |

### `static-analysis.yml` — Static analysis

| Job | Description |
|-----|-------------|
| **cppcheck** | Broad static analysis: undefined behaviour, memory issues, style and performance hints |
| **clang-tidy** | Compiler-level checks for bugs, portability and cert/bugprone categories |

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

## Scheduler Mock Library (`tests/scheduler_mock.c`)

Adds tiny artificial delays before `pthread_create` and `pthread_mutex_lock`
to force rare thread interleavings without causing failures.  Combined with
ThreadSanitizer this gives two independent layers of concurrency-bug coverage:

| Layer | What it does |
|-------|-------------|
| `scheduler_mock.so` | Nudges threads into unusual orderings via µs-scale delays |
| ThreadSanitizer | Detects unsynchronised memory accesses at the analysis level |

```sh
# Build the shared library
gcc -shared -fPIC -o libscheduler_mock.so tests/scheduler_mock.c -ldl

# Run philo with random delays (default: 0–500 µs per lock/create)
LD_PRELOAD=./libscheduler_mock.so ./philo 5 800 200 200

# Sequential mode – cycles through 10 evenly-spaced delay values
SCHED_MOCK_MODE=sequential LD_PRELOAD=./libscheduler_mock.so ./philo 5 800 200 200

# Reproducible run with explicit seed and verbose output
SCHED_MOCK_SEED=7 SCHED_MOCK_VERBOSE=1 LD_PRELOAD=./libscheduler_mock.so ./philo 5 800 200 200
```

| Variable | Default | Description |
|----------|---------|-------------|
| `SCHED_MOCK_MODE` | `random` | `random` or `sequential` delay strategy |
| `SCHED_MOCK_MAX_US` | `500` | Maximum delay in microseconds |
| `SCHED_MOCK_SEED` | `42` | PRNG seed (for reproducible random runs) |
| `SCHED_MOCK_VERBOSE` | unset | Set to `1` to log every injected delay to stderr |

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
│   ├── mock_posix.c               # LD_PRELOAD failure injection library
│   └── scheduler_mock.c           # LD_PRELOAD interleaving explorer (delays)
├── philo-test.py                  # Python test harness (5 axioms + stress)
└── .github/
    └── workflows/
        └── philo-ci.yml           # Full CI pipeline
```