*This project has been created as part of the 42 curriculum by syokota.*

# Codexion

## Description

Codexion is a concurrency exercise written in C with POSIX threads. Coders sit around a round table. Between each pair of neighbours there is one USB dongle, so the number of dongles equals the number of coders. To compile, a coder needs the two dongles next to them at the same time. A coder repeats the same cycle: take two dongles, compile, put them back, debug, refactor, then ask for the dongles again.

A coder burns out when they have not started a compile for `time_to_burnout` milliseconds, counted from the start of their last compile or from the start of the simulation. The simulation stops when a coder burns out or when every coder has compiled at least `number_of_compiles_required` times.

Each coder is a thread, and one more thread, the monitor, watches for burnouts. Dongles are shared resources protected by mutexes. When two coders want the same dongle, the dongle is given according to a scheduler: `fifo` (first come, first served) or `edf` (earliest deadline first, where the deadline is `last_compile_start + time_to_burnout`). After a dongle is released, it cannot be taken again before `dongle_cooldown` milliseconds have passed.

The program prints one line per state change:

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

## Instructions

### Build

```
make
```

This builds the `codexion` executable with `cc -Wall -Wextra -Werror -pthread`. Object files go to `obj/`. The Makefile also has the rules `clean` (remove `obj/`), `fclean` (remove `obj/` and `codexion`) and `re` (`fclean` then `all`). Running `make` twice in a row does not relink.

### Run

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coders and dongles. Must be at least 1. |
| `time_to_burnout` | Milliseconds a coder can go without starting a compile. |
| `time_to_compile` | Milliseconds a compile takes. The coder holds two dongles during it. |
| `time_to_debug` | Milliseconds spent debugging. |
| `time_to_refactor` | Milliseconds spent refactoring. |
| `number_of_compiles_required` | The simulation stops when every coder has compiled at least this number of times. |
| `dongle_cooldown` | Milliseconds a dongle stays unavailable after it is released. |
| `scheduler` | `fifo` or `edf`, written in lowercase. |

The seven numbers must be written with the digits 0 to 9 only and must fit in an `int` (at most 2147483647). A sign (`+` or `-`), spaces, a decimal point or an empty string is rejected. On a wrong argument the program prints one line on the standard error and exits with status 1 without starting the simulation:

```
$ ./codexion 4 800 200 200 200 5 0
Error: expected 8 arguments, got 7
$ ./codexion 0 800 200 200 200 5 0 fifo
Error: number_of_coders: '0' must be at least 1
$ ./codexion 4 -5 200 200 200 5 0 fifo
Error: time_to_burnout: '-5' is not a non-negative integer
$ ./codexion 4 3000000000 200 200 200 5 0 edf
Error: time_to_burnout: '3000000000' is too large (max 2147483647)
$ ./codexion 4 800 200 200 200 5 0 FIFO
Error: scheduler: 'FIFO' must be 'fifo' or 'edf'
```

If a mutex or a thread cannot be created, or memory cannot be allocated, the program prints one line such as `Error: failed to create coder thread`, cleans up what it created, and exits with status 1. A simulation that ends, with or without a burnout, exits with status 0. With `number_of_compiles_required` set to 0 the program prints nothing and exits with status 0, because every coder has already compiled zero times.

### Examples

The order of lines with the same timestamp changes from run to run. These are outputs of single runs on my machine.

Every coder compiles once, nobody burns out:

```
$ ./codexion 4 800 200 100 100 1 0 edf
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 3 is debugging
200 1 is debugging
200 4 has taken a dongle
200 4 has taken a dongle
200 4 is compiling
200 2 has taken a dongle
200 2 has taken a dongle
200 2 is compiling
300 3 is refactoring
300 1 is refactoring
402 2 is debugging
402 4 is debugging
402 3 has taken a dongle
402 3 has taken a dongle
402 3 is compiling
```

A setting that cannot be satisfied: coders 2 and 4 compile until 401, but coder 1 has to start again before 310.

```
$ ./codexion 4 310 200 100 100 5 0 fifo
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 3 is debugging
200 4 has taken a dongle
200 1 is debugging
201 2 has taken a dongle
201 2 has taken a dongle
201 2 is compiling
201 4 has taken a dongle
201 4 is compiling
300 1 is refactoring
300 3 is refactoring
310 1 burned out
```

One coder has only one dongle, so they can never compile:

```
$ ./codexion 1 800 200 200 200 5 0 fifo
0 1 has taken a dongle
800 1 burned out
```

## How the program runs

1. `parse.c` checks the eight arguments.
2. If `number_of_compiles_required` is 0, `main` returns at once.
3. `init.c` allocates the dongle and coder arrays and initialises every mutex. Each successful step is counted, so `cleanup.c` destroys and frees only what was created.
4. `main.c` records the start time, then creates one thread per coder and the monitor thread.
5. Each coder thread (`coder.c`) loops: take two dongles (`dongle.c`), compile, put the dongles back, debug, refactor.
6. The monitor thread (`monitor.c`) checks every coder about once per millisecond. It stops the simulation on a burnout or when every coder has compiled enough times.
7. Every waiting loop checks the stop flag, so all threads return. `main` joins them, destroys the mutexes and frees the arrays.

All times are milliseconds since the start of the simulation, taken with `gettimeofday`. Log timestamps, deadlines and dongle cooldowns use the same clock.

Source files:

| File | Role |
|---|---|
| `main.c` | Overall flow, thread creation and join |
| `parse.c` | Argument checks and number conversion |
| `init.c`, `cleanup.c` | Allocation and mutex setup, and the reverse |
| `coder.c` | Coder thread: compile, debug, refactor |
| `dongle.c` | Taking and releasing dongles, cooldown |
| `heap.c` | Priority queue (binary heap) of waiting coders |
| `scheduler.c` | FIFO and EDF ordering, ticket numbers |
| `monitor.c` | Burnout detection and the stop condition |
| `log.c` | Log output and the stop flag |
| `time.c` | Clock and a sleep that can be interrupted |

## Blocking cases handled

### Deadlock

A deadlock needs four conditions at the same time (Coffman, 1971): mutual exclusion, hold and wait, no preemption, and circular wait. A dongle can only be used by one coder, a coder holds the first dongle while waiting for the second one, and nobody takes a dongle away from a coder. So the first three conditions are present, and the program removes the fourth one, circular wait.

Coders take their dongles one at a time in a fixed order. A coder with an odd number takes the right dongle first. A coder with an even number takes the left dongle first. With this rule, two neighbours always start with the dongle they share. For example, the right dongle of coder 1 and the left dongle of coder 2 are both dongle 1 (index starting at 0). Only one of them can get it. The other one holds nothing while waiting. A circle where every coder holds one dongle and waits for the next one would need two neighbours that each hold a different first dongle, and that cannot happen.

The mutexes themselves cannot deadlock either. A coder locks at most one dongle mutex at a time. The other locks are always taken in the same order: coder mutex, then log mutex, then stop mutex.

### Waiting chains

Avoiding deadlock is not enough. If waiting coders block each other in a long line, only one coder compiles at a time and coders burn out even when the parameters allow everyone to survive. The odd/even order keeps such lines short. When a coder holds their first dongle and waits for the second one, that second dongle is also the second dongle of the neighbour on the other side. Someone holding it already has both dongles and is compiling, so it comes back after one compile and one cooldown, and the line of waiting coders does not go further than that neighbour. With an odd number of coders, dongle 0 is the first dongle for one neighbour and the second dongle for the other, and there the line can be one coder longer.

### Fair arbitration and starvation

Each dongle has its own priority queue of waiting coders, stored as a binary heap. When a coder asks for dongles, they get a ticket number from a shared counter and enter the queues of both of their dongles at once. A coder takes their first dongle only when they are at the head of its queue.

- With `fifo`, the queue is ordered by ticket number, so the coder who asked first comes first.
- With `edf`, the queue is ordered by deadline (`last_compile_start + time_to_burnout`). When two deadlines are equal, the smaller ticket number wins. Ticket numbers are never equal, so the order is always fully decided.

The second dongle follows the same rule with one exception. A coder who already holds their first dongle may take the second one before the coder at the head of its queue, if the head coder's deadline is later than `now + time_to_compile + dongle_cooldown`. Taking the second dongle starts a compile at once, so the dongle comes back after one compile and one cooldown, and the head coder can still use it before their deadline. Without this exception, a coder who is still waiting for their first dongle blocks a free second dongle, and the waiting chains described above come back. The exception also applies with `fifo`, so `fifo` follows arrival order except in this case.

Entering both queues at the start matters for `edf`. In an earlier version a coder joined the queue of their second dongle only after getting the first one. A coder with a close deadline was then invisible in that queue while waiting for their first dongle, and a coder with a later deadline could take the second dongle first. With `3 650 200 100 100 20 0 edf` (three coders need 600 ms per round, so there are 50 ms of slack) that version burned out in 5 runs out of 5. The current version survived 5 runs out of 5.

The subject asks for liveness under `edf` when the parameters are feasible. I do not have a proof for every feasible setting. The settings I ran, five runs each unless written otherwise, are listed in [Limits](#limits).

### Cooldown

Each dongle stores `available_at`, the time when it can be used again. Releasing a dongle sets `available_at = now + dongle_cooldown`. A coder can take a dongle only if `now >= available_at`. The value is read and written under the dongle mutex, together with the queue and the `in_use` flag, so the three checks happen at one point in time.

### Burnout detection

Each coder has a `last_compile_start` value and a mutex protecting it. The coder writes it when a compile starts. The monitor reads it about once per millisecond and reports a burnout when `now - last_compile_start >= time_to_burnout`. The subject requires the burnout line within 10 ms of the real burnout. In my runs (seven settings that end in a burnout, five runs each) the line was printed 0 or 1 ms after the deadline.

The monitor keeps the coder mutex locked from the check until the burnout line is printed. Otherwise the coder could start a compile between the check and the print, and the log would show `is compiling` right before `burned out` for the same coder. With the lock held, the coder waits on its own mutex, and when it continues the stop flag is already set and its line is not printed.

### Log serialization

Every state line goes through `log_state` in `log.c`. It locks the log mutex, checks the stop flag, prints the line with `printf` if the simulation is still running, and unlocks. Two lines can never be mixed, and no line appears after `burned out`, because the monitor sets the stop flag inside the same log mutex before printing `burned out`. The timestamp is read after the log mutex is taken, so printed timestamps never go backwards.

### Stop and cleanup

Coders do not get woken up by anyone. Every waiting loop and every sleep checks the stop flag every 0.5 ms and returns when it is set. The main thread then joins all threads and destroys the mutexes. A coder that is still in a dongle queue when the simulation stops leaves without removing itself, because the queues are freed with the dongles.

### Special cases

- One coder: there is only one dongle, so the coder prints `has taken a dongle` once and waits until the monitor reports the burnout.
- `number_of_compiles_required` is 0: the program exits before creating any thread.
- `time_to_burnout` is 0: the first check of the monitor reports a burnout.
- `time_to_compile`, `time_to_debug`, `time_to_refactor` or `dongle_cooldown` is 0: no special handling, the wait lasts zero milliseconds.

## Thread synchronization mechanisms

### Threads

A thread is a flow of execution inside a process. All threads of a process share the same memory, which is why they can share the dongles and the log, and also why that shared memory needs protection. Each coder is one thread created with `pthread_create`, as the subject requires.

The monitor is a separate thread. A coder cannot watch its own burnout while it is blocked waiting for a dongle or sleeping, and it does not know the state of other coders. A thread that only watches can report a burnout at the right time whatever the coders are doing.

### Mutexes

A mutex (`pthread_mutex_t`) lets one thread at a time run the code between `pthread_mutex_lock` and `pthread_mutex_unlock`. The program uses five kinds:

| Mutex | Count | Protects |
|---|---|---|
| Dongle mutex | one per dongle | `in_use`, `available_at` and the waiting queue of that dongle |
| Coder mutex | one per coder | `last_compile_start` and `compile_count` of that coder |
| `log_mutex` | one | standard output |
| `stop_mutex` | one | `stop_flag` |
| `ticket_mutex` | one | the ticket counter `next_ticket` |

The coder mutexes are separate so that the monitor reading one coder does not stop the others. The stop flag has its own mutex because coders read it every 0.5 ms, and these reads should not wait behind log output.

### Shared data

| Data | Written by | Read by | Protected by |
|---|---|---|---|
| Dongle `in_use`, `available_at`, queue | coders | coders | dongle mutex |
| `last_compile_start` | its coder | its coder, monitor | coder mutex |
| `compile_count` | its coder | monitor | coder mutex |
| `stop_flag` | monitor, main thread on a creation error | everyone | `stop_mutex` |
| `next_ticket` | coders | coders | `ticket_mutex` |
| Standard output | coders, monitor | | `log_mutex` |
| Arguments, start time | main thread before the threads start | everyone | nothing, they never change after the threads start |

### Races that are prevented

- Two neighbours want the same free dongle: both lock the dongle mutex, so the check of `in_use` and the write `in_use = 1` happen together for one coder, and the other one sees `in_use == 1`.
- Two coders ask for dongles at the same moment: the ticket counter is increased under `ticket_mutex`, so they get different numbers.
- A coder starts a compile while the monitor checks it: both use the coder mutex, so the monitor sees either the old or the new `last_compile_start`, never a half-written value.

### Communication between coders and the monitor

Coders and the monitor do not send messages to each other. They share two values per coder and the stop flag.

1. A coder starts a compile: it locks its coder mutex, writes `last_compile_start = now`, unlocks, and prints `is compiling`.
2. The monitor locks the same mutex and computes `now - last_compile_start`.
3. If it is a burnout, the monitor, still holding the coder mutex, locks `log_mutex`, sets `stop_flag` under `stop_mutex`, prints `burned out` and unlocks.
4. Every coder sees `stop_flag` at its next check, at most 0.5 ms later, and returns.

When a coder finishes a compile, it increases `compile_count` under its mutex. When every count reaches `number_of_compiles_required`, the monitor sets the stop flag under `log_mutex` without printing anything.

### Why there are no condition variables

The subject allows `pthread_cond_t` for waiting queues. I used short polling instead: a waiting coder unlocks, sleeps 0.5 ms with `usleep`, locks again and checks. A coder can be blocked for three different reasons (the dongle is in use, the cooldown is not over, another coder has priority), and a single loop checks all three plus the stop flag. With condition variables the program would need `pthread_cond_timedwait` with an absolute time for the cooldown, a broadcast on every release, and a broadcast to every dongle when the simulation stops. Each missing broadcast would leave a thread sleeping. Polling costs more CPU time, which I accepted.

## Design choices and values

### Why a heap with only two coders

At most two coders, the two neighbours, ever wait for the same dongle, so each heap has a fixed capacity of 2 and lives inside the dongle structure (no allocation). The subject requires a priority queue for the scheduler, and the heap gives one structure for both policies: `has_priority` in `scheduler.c` is the only part that differs between `fifo` and `edf`. The heap can remove an element that is not at the head, which the overtaking rule needs.

### Why ticket numbers

FIFO could use the request time as the key, but two requests can happen in the same millisecond (at the start of the simulation every coder asks at 0 ms), and a tie broken by coder number would always favour the same coders. A global counter gives every request a different number, and the same number is used in both queues of a coder.

### Why 0.5 ms and 1 ms

`POLL_INTERVAL_US` (500) is the sleep between two checks of a waiting coder and the step of the interruptible sleep. `MONITOR_INTERVAL_US` (1000) is the pause between two rounds of the monitor. One millisecond is well inside the 10 ms allowed for the burnout message. I also tried polling at 250 and 100 microseconds on three settings with 10 ms of slack, ten runs each. This was measured with the previous dongle rule (design 3 below). A round took 400 to 407 ms at 500 microseconds and 400 to 404 ms at 100 microseconds, but the number of burnouts did not go down (0 out of 30 at 500, 1 out of 30 at 250 and at 100). Those burnouts came from the operating system waking a thread late, which a shorter interval does not fix, so I kept 500.

### Why `gettimeofday` and milliseconds

The subject recommends `gettimeofday`, and every argument and log timestamp is in milliseconds. All times are stored as `long long` milliseconds since the start, so adding a deadline to a large `time_to_burnout` cannot overflow.

### When the compile count increases

`compile_count` increases when the compile wait is over, before the dongles go back. The subject says the simulation stops when every coder "has compiled" enough times, so a compile counts once it is finished. `last_compile_start` is written when the compile starts, because burnout is about starting a compile. The coder prints `is debugging` before releasing the dongles. If it released first, a neighbour could print `is compiling` before this coder's `is debugging`, and the log would look like two neighbours compiling together.

### Designs I tried and dropped

All four were measured with five runs per setting.

1. Wait without holding anything until both dongles are free and the coder is at the head of both queues, then take both at once. This removes hold and wait, but a coder waiting at the head of one queue blocked a free dongle for the neighbour on the other side, and those waits chained around the table. With `5 800 200 200 200 edf` only one coder compiled at a time, and the gap between two compiles of one coder reached 1001 ms.
2. Take dongles one at a time, lower index first, and join the second queue after getting the first dongle. With 200 coders (`200 1000 100 100 100`) coders holding one dongle waited for each other in long lines. The longest gap between two compiles of one coder was 1502 ms, fifteen compile times, and 3 runs out of 5 burned out.
3. The odd/even order, still joining the second queue late. Large tables worked, but `3 650 200 100 100 edf` burned out 5 times out of 5, as explained in [Fair arbitration and starvation](#fair-arbitration-and-starvation).
4. Join both queues at once, with no overtaking. The three-coder case worked, but the waiting chains of design 1 came back: `5 800`, `4 410` and 200 coders burned out in every run.

## Limits

These results come from macOS on my machine. The 42 computers run Linux, so the numbers there can differ.

Settings that did not burn out in any of five runs: `3 650 200 100 100 edf`, `3 700 200 100 100` with a cooldown of 20 (`edf`), `3 800 200 200 200 fifo`, `5 610 200 100 100 edf`, `5 800 200 200 200` (`edf` and `fifo`), `4 410 200 100 100 fifo`, `4 410 200 200 0 edf`, `7 1000 200 100 100` with a cooldown of 10 (`edf`), `199 1000 100 100 100 edf` and `200 1000 100 100 100 edf`.

`5 650 200 100 100 edf` (50 ms of slack) burned out in 2 runs out of 5 in one series. A second series of 40 runs had no burnout. I did not find the cause of the first two.

Settings with about 10 ms of slack can burn out. `4 410 200 100 100 edf` burned out in 3 runs out of 5 and `4 450 200 100 100` with a cooldown of 20 in 2 runs out of 5. In one of these logs a debug phase of 100 ms took 122 ms because the thread was woken late, and a 10 ms margin cannot absorb that.

`fifo` ignores deadlines, so it can let a coder burn out where `edf` would not. The subject asks for liveness only under `edf`.

## Resources

- POSIX manual pages: `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_lock(3)`, `gettimeofday(2)`, `usleep(3)`
- E. G. Coffman, M. J. Elphick, A. Shoshani, "System Deadlocks", ACM Computing Surveys 3(2), 1971 (the four conditions for deadlock)
- E. W. Dijkstra, "Hierarchical ordering of sequential processes" (EWD310), 1971 (the dining philosophers problem, which Codexion is based on)
- C. L. Liu, J. W. Layland, "Scheduling Algorithms for Multiprogramming in a Hard-Real-Time Environment", Journal of the ACM 20(1), 1973 (earliest deadline first)
- T. H. Cormen, C. E. Leiserson, R. L. Rivest, C. Stein, "Introduction to Algorithms", chapter 6 (binary heaps and priority queues)
- Clang documentation, ThreadSanitizer (used to look for data races while testing)

### Use of AI

I used Claude Code (Anthropic) in these parts of the project:

- Design: discussing options for each design decision. The decisions and the rejected options were kept in a design memo, and each change was confirmed by me before the code changed.
- Code: writing the code one step at a time (header, argument check, setup and cleanup, heap and dongles, coders, monitor, log and time, Makefile). I reviewed each step before the next one.
- Testing: writing throwaway test programs that are not in this repository (stand-in functions for parts not yet written, log checkers, timing measurements, ThreadSanitizer and `leaks` runs). The measurements in this README come from those runs.
- Documentation: a first draft of this README, which I checked against the code.
