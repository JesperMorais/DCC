## The bike computer that froze mid-ride

A GPS bike computer runs two tasks that share two resources, each protected by its own mutex:

| task | prio | starts | each round |
|---|---|---|---|
| `nav` | 2 | tick `nav_start_delay` (1 by default) | lock `gps_lock`, read a fix (2 ticks), lock `sd_lock`, `track_append()` (1 tick), unlock both, sleep 3 ticks |
| `archive` | 1 | tick 0 | lock `sd_lock`, open the archive file (2 ticks), lock `gps_lock`, `archive_stamp()` (stamp it with the GPS time, 1 tick), unlock both, sleep 2 ticks |

Each task does `ROUNDS` (5) rounds and then returns. `track_append()` and `archive_stamp()` are provided by the board (the tests).

Riders report that the screen sometimes freezes a few seconds into a ride, and only a power cycle brings it back. On the bench it "usually works". Run the tests and open the Timeline: with the default start-up timing, both tasks are blocked forever by tick 4.

**Fix it so that, over 300 ticks:**

1. **There's no deadlock** (`sim_deadlocked()` is false), with nav starting at tick 0, 1, 2 or 3. A fix that happens to dodge one start-up timing doesn't count: it has to be impossible, not unlikely.
2. **Both tasks finish all their rounds:** `fixes_logged` and `files_archived` both reach `ROUNDS`, and both tasks return.
3. **Both locks are free at the end.** Nobody leaves a mutex locked.
4. **The shared things stay protected.** Every call to `track_append()` and `archive_stamp()` happens while the caller owns *both* `gps_lock` and `sd_lock`. Dropping a lock would make the deadlock go away and the data race come back.

Don't change `bike_start`, the priorities, the work durations or the delays, and keep the variable names (`gps_lock`, `sd_lock`, `fixes_logged`, `files_archived`, `nav_start_delay`). The tests set `nav_start_delay` before calling `bike_start()`.

On the Timeline, look for the `wait` events on the two mutexes: before the fix, each task waits for the lock the other one holds. You will even see `archive` inherit priority 2 at tick 3: priority inheritance is on, and it does nothing for a cycle. After the fix, you'll see one task simply wait its turn.
