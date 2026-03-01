#!/usr/bin/env python3
"""
philo-test.py
=============
Automated test harness for the 42 "philosophers" project.

It validates the 5 classic dining-philosophers axioms:
  1. Resources   – forks (mutexes) are never shared concurrently
  2. Time        – timestamps are monotonic and in milliseconds
  3. Death       – a philosopher that hasn't eaten within time_to_die ms dies
  4. Progress    – when no philosopher should die, they keep eating indefinitely
  5. Fairness    – every philosopher gets to eat (no starvation)

Usage:
  python3 philo-test.py [--binary ./philo] [--timeout 30]

Exit code: 0 = all tests passed, 1 = at least one test failed.
"""

import argparse
import re
import subprocess
import sys
import time
from typing import List, Optional, Tuple


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

ANSI_GREEN = "\033[32m"
ANSI_RED   = "\033[31m"
ANSI_CYAN  = "\033[36m"
ANSI_RESET = "\033[0m"


def ok(msg: str) -> None:
    print(f"  {ANSI_GREEN}[PASS]{ANSI_RESET} {msg}")


def fail(msg: str) -> None:
    print(f"  {ANSI_RED}[FAIL]{ANSI_RESET} {msg}")


def info(msg: str) -> None:
    print(f"  {ANSI_CYAN}[INFO]{ANSI_RESET} {msg}")


def run_philo(
    binary: str,
    args: List[str],
    timeout: float,
    env: Optional[dict] = None,
) -> Tuple[int, str, str]:
    """Run the philosopher binary and return (returncode, stdout, stderr)."""
    cmd = [binary] + [str(a) for a in args]
    info(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            env=env,
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired as exc:
        # Timeout is expected for "must not die" tests – collect partial output
        stdout = exc.stdout.decode(errors="replace") if exc.stdout else ""
        stderr = exc.stderr.decode(errors="replace") if exc.stderr else ""
        return -1, stdout, stderr


def parse_log_lines(output: str) -> List[dict]:
    """
    Parse philosopher log lines of the form:
      <timestamp_ms> <id> <action>
    Returns a list of dicts with keys: ts (int), id (int), action (str).
    """
    events = []
    pattern = re.compile(r"^\s*(\d+)\s+(\d+)\s+(.+)$")
    for line in output.splitlines():
        m = pattern.match(line)
        if m:
            events.append(
                {"ts": int(m.group(1)), "id": int(m.group(2)), "action": m.group(3).strip()}
            )
    return events


# ---------------------------------------------------------------------------
# Individual axiom validators
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# PBT invariant validators (A–D from the evaluation sheet)
# ---------------------------------------------------------------------------


def invariant_death_precision(events: List[dict], time_to_die: int) -> bool:
    """
    Invariant A – Death Precision: the 'died' message timestamp must not
    exceed (last_meal_ts + time_to_die + 10 ms).  If the philosopher never
    ate, the simulation start time (first event) is used as the reference.
    """
    if not events:
        ok("Invariant A (Death Precision): no events to check")
        return True

    start_ts = events[0]["ts"]
    last_meal: dict = {}  # philo_id -> timestamp of most recent eating event
    passed = True

    for ev in events:
        pid = ev["id"]
        act = ev["action"].lower()
        if "eating" in act:
            last_meal[pid] = ev["ts"]
        elif "died" in act:
            ref_ts = last_meal.get(pid, start_ts)
            deadline = ref_ts + time_to_die + 10
            if ev["ts"] > deadline:
                fail(
                    f"Invariant A (Death Precision): philo {pid} death at "
                    f"ts={ev['ts']} exceeds deadline {deadline} "
                    f"(last_meal={ref_ts}, time_to_die={time_to_die})"
                )
                passed = False

    if passed:
        ok("Invariant A (Death Precision): all death timestamps within 10 ms tolerance")
    return passed


def invariant_no_ghost_actions(events: List[dict]) -> bool:
    """
    Invariant B – No Ghost Actions: once a 'died' message is logged, no
    other status messages with a strictly later timestamp may appear.
    """
    death_events = [ev for ev in events if "died" in ev["action"].lower()]
    if not death_events:
        ok("Invariant B (No Ghost Actions): no deaths – nothing to check")
        return True

    first_death_ts = min(ev["ts"] for ev in death_events)
    passed = True

    for ev in events:
        if "died" in ev["action"].lower():
            continue
        if ev["ts"] > first_death_ts:
            fail(
                f"Invariant B (No Ghost Actions): '{ev['action']}' by philo "
                f"{ev['id']} at ts={ev['ts']} appears after first death at "
                f"ts={first_death_ts}"
            )
            passed = False
            break

    if passed:
        ok("Invariant B (No Ghost Actions): no events logged after first death")
    return passed


def invariant_fork_exclusivity(events: List[dict], num_philos: int) -> bool:
    """
    Invariant C – Fork Exclusivity: at any given moment a fork cannot be held
    by two philosophers.  Adjacent philosophers (i and i±1 mod N) share a
    fork, so they must never eat simultaneously.
    """
    eating: set = set()  # philosopher ids currently in the eating state
    passed = True

    for ev in events:
        pid = ev["id"]
        act = ev["action"].lower()
        if "eating" in act:
            left = (pid - 2) % num_philos + 1   # left neighbour
            right = pid % num_philos + 1          # right neighbour
            for neighbour in (left, right):
                if neighbour in eating:
                    fail(
                        f"Invariant C (Fork Exclusivity): philo {pid} and philo "
                        f"{neighbour} eat simultaneously at ts={ev['ts']} – fork stolen"
                    )
                    passed = False
            eating.add(pid)
        elif "sleeping" in act:
            eating.discard(pid)

    if passed:
        ok("Invariant C (Fork Exclusivity): no fork held by two philosophers simultaneously")
    return passed


def invariant_completion(events: List[dict], num_philos: int, must_eat: int) -> bool:
    """
    Invariant D – Completion: when the must-eat argument is used, every
    philosopher must eat at least `must_eat` times and no philosopher should
    die (assuming safe timing parameters).
    """
    eat_counts: dict = {}
    for ev in events:
        if "eating" in ev["action"].lower():
            eat_counts[ev["id"]] = eat_counts.get(ev["id"], 0) + 1

    passed = True
    for pid in range(1, num_philos + 1):
        count = eat_counts.get(pid, 0)
        if count < must_eat:
            fail(
                f"Invariant D (Completion): philo {pid} ate only {count}/{must_eat} times"
            )
            passed = False

    deaths = [ev for ev in events if "died" in ev["action"].lower()]
    if deaths:
        fail(
            f"Invariant D (Completion): philo {deaths[0]['id']} died unexpectedly "
            f"at ts={deaths[0]['ts']} during must-eat scenario"
        )
        passed = False

    if passed:
        ok(
            f"Invariant D (Completion): all {num_philos} philosophers ate "
            f"≥{must_eat} times with no deaths"
        )
    return passed


def axiom_resources(events: List[dict], num_philos: int) -> bool:
    """
    Axiom 1 – Resources: check that no two philosophers hold the same fork
    simultaneously.  We infer fork ownership from 'has taken a fork' /
    'is eating' / 'is sleeping' state transitions.  A simplified check
    verifies that no philosopher picks up more than 2 forks at a time.
    """
    held: dict = {}  # philo_id -> fork_count
    passed = True
    for ev in events:
        pid = ev["id"]
        act = ev["action"].lower()
        if "taken a fork" in act or "has taken a fork" in act:
            held[pid] = held.get(pid, 0) + 1
            if held[pid] > 2:
                fail(f"Axiom 1 (Resources): philo {pid} holds >2 forks at ts={ev['ts']}")
                passed = False
        elif "sleeping" in act:
            # Philosophers release both forks once they start sleeping.
            # Using 'sleeping' (not 'eating') as the release point matches the
            # typical state machine: think → take fork → take fork → eat → sleep.
            held[pid] = 0
    if passed:
        ok("Axiom 1 (Resources): no philosopher holds more than 2 forks")
    return passed


def axiom_time(events: List[dict]) -> bool:
    """
    Axiom 2 – Time: timestamps must be non-decreasing (monotonic).
    """
    passed = True
    prev_ts = -1
    for ev in events:
        if ev["ts"] < prev_ts:
            fail(
                f"Axiom 2 (Time): timestamp decreased from {prev_ts} to {ev['ts']} "
                f"at event: {ev}"
            )
            passed = False
            break
        prev_ts = ev["ts"]
    if passed:
        ok("Axiom 2 (Time): all timestamps are monotonically non-decreasing")
    return passed


def axiom_death(
    events: List[dict],
    time_to_die: int,
    expect_death: bool,
) -> bool:
    """
    Axiom 3 – Death: if a philosopher should die, the 'died' message must
    appear.  If no philosopher should die, no 'died' message should appear.
    """
    deaths = [ev for ev in events if "died" in ev["action"].lower()]
    if expect_death:
        if deaths:
            ok(f"Axiom 3 (Death): philosopher died as expected (ts={deaths[0]['ts']})")
            return True
        else:
            fail("Axiom 3 (Death): expected a death but none was reported")
            return False
    else:
        if deaths:
            fail(
                f"Axiom 3 (Death): unexpected death of philo {deaths[0]['id']} "
                f"at ts={deaths[0]['ts']}"
            )
            return False
        ok("Axiom 3 (Death): no unexpected deaths detected")
        return True


def axiom_progress(events: List[dict]) -> bool:
    """
    Axiom 4 – Progress: at least one philosopher must eat during a run
    (ensures the simulation is not frozen).
    """
    eaters = {ev["id"] for ev in events if "eating" in ev["action"].lower()}
    if eaters:
        ok(f"Axiom 4 (Progress): philosophers that ate: {sorted(eaters)}")
        return True
    fail("Axiom 4 (Progress): no philosopher ate during the run")
    return False


def axiom_fairness(events: List[dict], num_philos: int) -> bool:
    """
    Axiom 5 – Fairness: every philosopher must eat at least once.
    Only validated when the simulation runs long enough (progress test).
    """
    eaters = {ev["id"] for ev in events if "eating" in ev["action"].lower()}
    missing = set(range(1, num_philos + 1)) - eaters
    if missing:
        fail(f"Axiom 5 (Fairness): philosopher(s) never ate: {sorted(missing)}")
        return False
    ok(f"Axiom 5 (Fairness): all {num_philos} philosophers ate at least once")
    return True


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_single_philosopher_dies(binary: str, timeout: float) -> bool:
    """
    1 philosopher, 800ms to die, 200ms to eat, 200ms to sleep.
    The single philosopher cannot eat (only one fork) and must die.
    """
    print("\n[Test] Single philosopher must die")
    _, stdout, _ = run_philo(binary, [1, 800, 200, 200], timeout)
    events = parse_log_lines(stdout)
    results = [
        axiom_time(events),
        axiom_death(events, time_to_die=800, expect_death=True),
        invariant_death_precision(events, time_to_die=800),
        invariant_no_ghost_actions(events),
    ]
    return all(results)


def test_no_death(binary: str, timeout: float) -> bool:
    """
    5 philosophers, 800ms to die, 200ms to eat, 200ms to sleep.
    No philosopher should die; all should eat.
    """
    print("\n[Test] No philosopher should die (5 philos, enough time)")
    # Run for a bounded time then check partial output
    rc, stdout, _ = run_philo(binary, [5, 800, 200, 200], min(timeout, 5.0))
    events = parse_log_lines(stdout)
    results = [
        axiom_resources(events, 5),
        axiom_time(events),
        axiom_death(events, time_to_die=800, expect_death=False),
        axiom_progress(events),
        axiom_fairness(events, 5),
        invariant_fork_exclusivity(events, 5),
        invariant_no_ghost_actions(events),
    ]
    return all(results)


def test_must_eat_count(binary: str, timeout: float) -> bool:
    """
    5 philosophers with must-eat count of 3.
    The simulation should stop after each philosopher has eaten 3 times.
    """
    print("\n[Test] Simulation stops after each philo eats 3 times")
    rc, stdout, _ = run_philo(binary, [5, 800, 200, 200, 3], timeout)
    events = parse_log_lines(stdout)
    eat_counts: dict = {}
    for ev in events:
        if "eating" in ev["action"].lower():
            eat_counts[ev["id"]] = eat_counts.get(ev["id"], 0) + 1

    passed = True
    results = [
        axiom_resources(events, 5),
        axiom_time(events),
        axiom_death(events, time_to_die=800, expect_death=False),
        axiom_progress(events),
        invariant_fork_exclusivity(events, 5),
        invariant_no_ghost_actions(events),
        invariant_completion(events, num_philos=5, must_eat=3),
    ]
    passed = all(results)
    return passed


def test_tight_timing(binary: str, timeout: float) -> bool:
    """
    4 philosophers with very tight timing (310ms to die, 200ms to eat,
    100ms to sleep).  This is a common edge-case in 42 evaluations.
    """
    print("\n[Test] Tight timing (4 philos, 310ms die, 200ms eat, 100ms sleep)")
    rc, stdout, _ = run_philo(binary, [4, 310, 200, 100], min(timeout, 8.0))
    events = parse_log_lines(stdout)
    results = [
        axiom_resources(events, 4),
        axiom_time(events),
        axiom_death(events, time_to_die=310, expect_death=False),
        axiom_progress(events),
        axiom_fairness(events, 4),
        invariant_fork_exclusivity(events, 4),
        invariant_no_ghost_actions(events),
    ]
    return all(results)


def test_invalid_args(binary: str, timeout: float) -> bool:
    """
    The program must exit with a non-zero code when given invalid arguments.
    """
    print("\n[Test] Invalid arguments must return non-zero exit code")
    passed = True
    # Too few arguments
    rc, _, _ = run_philo(binary, [], timeout)
    if rc == 0:
        fail("Invalid args: empty args returned exit code 0")
        passed = False
    else:
        ok(f"Empty args: exit code {rc} (non-zero, expected)")
    # Zero philosophers
    rc, _, _ = run_philo(binary, [0, 800, 200, 200], timeout)
    if rc == 0:
        fail("Invalid args: 0 philosophers returned exit code 0")
        passed = False
    else:
        ok(f"0 philosophers: exit code {rc} (non-zero, expected)")
    return passed


# ---------------------------------------------------------------------------
# Stress loop
# ---------------------------------------------------------------------------

def stress_loop(binary: str, iterations: int, timeout: float) -> bool:
    """
    Run the no-death scenario multiple times to catch race conditions.
    Returns True if all iterations pass.
    """
    print(f"\n[Stress] Running {iterations} iterations of no-death test …")
    passed = True
    for i in range(1, iterations + 1):
        rc, stdout, _ = run_philo(binary, [5, 800, 200, 200], min(timeout, 4.0))
        events = parse_log_lines(stdout)
        deaths = [ev for ev in events if "died" in ev["action"].lower()]

        # Check for unexpected deaths
        if deaths:
            fail(f"  Stress iteration {i}/{iterations}: unexpected death detected")
            passed = False
        else:
            # Also check for ghost actions even if no deaths in this iteration
            if not invariant_no_ghost_actions(events):
                fail(f"  Stress iteration {i}/{iterations}: ghost actions detected")
                passed = False
            else:
                info(f"  Iteration {i}/{iterations}: OK ({len(events)} events)")
    if passed:
        ok(f"Stress loop: all {iterations} iterations passed without unexpected deaths or ghost actions")
    return passed


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(description="Philosopher project test suite")
    parser.add_argument(
        "--binary", default="./philo", help="Path to the philo binary (default: ./philo)"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=30.0,
        help="Per-test timeout in seconds (default: 30)",
    )
    parser.add_argument(
        "--stress-iterations",
        type=int,
        default=5,
        help="Number of stress-loop iterations (default: 5)",
    )
    args = parser.parse_args()

    print(f"{ANSI_CYAN}{'='*60}")
    print("  ft_philosophers – automated test suite")
    print(f"  Binary : {args.binary}")
    print(f"  Timeout: {args.timeout}s per test")
    print(f"{'='*60}{ANSI_RESET}\n")

    # -----------------------------------------------------------------------
    # Run all test cases
    # -----------------------------------------------------------------------
    suite = [
        test_invalid_args(args.binary, args.timeout),
        test_single_philosopher_dies(args.binary, args.timeout),
        test_no_death(args.binary, args.timeout),
        test_must_eat_count(args.binary, args.timeout),
        test_tight_timing(args.binary, args.timeout),
        stress_loop(args.binary, args.stress_iterations, args.timeout),
    ]

    # -----------------------------------------------------------------------
    # Summary
    # -----------------------------------------------------------------------
    passed = sum(suite)
    total  = len(suite)
    print(f"\n{ANSI_CYAN}{'='*60}")
    print(f"  Results: {passed}/{total} test groups passed")
    print(f"{'='*60}{ANSI_RESET}")

    return 0 if all(suite) else 1


if __name__ == "__main__":
    sys.exit(main())
