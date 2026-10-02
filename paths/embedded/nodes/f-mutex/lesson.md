### Mars, 1997: the rover that kept rebooting

On 4 July 1997, Mars Pathfinder bounced onto Mars in its airbags and started sending back pictures. A few days later, the spacecraft began to **reset itself**, losing a little of each day's work every time. The software ran on VxWorks, and the JPL team ran an identical replica in the lab for about 18 hours, with full tracing on, before it reproduced the bug. When they caught it, the trace showed a textbook case:

- A **low**-priority meteorology task took the mutex protecting the shared "information bus".
- The **high**-priority bus-management task needed that mutex, and blocked.
- A **medium**-priority communications task, which didn't need the bus at all, became ready and ran for a long time. It preempted the low task, so the low task couldn't finish and release the mutex.
- The high task waited... and waited. A **watchdog** saw that the bus-management cycle hadn't completed, concluded the system was hung, and reset the whole computer.

The fix was one flag. The mutex had been created with **priority inheritance off**. VxWorks shipped with a C interpreter on board, so JPL uploaded a tiny patch that flipped it on. The resets stopped. Glenn Reeves, who led the software team, later wrote that the same reset had shown up a few times in pre-launch testing, never reproduced, and been put down to hardware glitches.

### Mutex vs semaphore: who owns the lock?

You met semaphores as *signals*: anyone gives, anyone takes. A **mutex** looks similar (lock/unlock), but it has an **owner**:

| | semaphore | mutex |
|---|---|---|
| purpose | signal "something happened" / count resources | mutual exclusion around shared data |
| who releases | anyone, including an ISR | **only the task that locked it** |
| from an ISR | give: yes | never |
| priority inheritance | no (who would inherit?) | yes, because the kernel knows the owner |

That ownership is the whole trick. Because the kernel knows *who* holds the mutex, it can do something about the holder when someone important is waiting. And it can catch "you unlocked a mutex you don't own" bugs (our simulator does).

### Priority inversion, step by step

The high task is stuck behind the low one. That part is **bounded**: it's just the length of low's critical section, and you accept it when you share a resource. The disaster is the medium task. It doesn't use the bus, but it preempts *low*, so it effectively blocks *high* for as long as it runs. Add more medium tasks, and the wait has no bound at all: that's **unbounded priority inversion**.

```
tick           0    5    10   15   20
high   (3)       X............##      X = wants the bus, . = blocked
medium (2)        ##########          doesn't even use the bus
low    (1)     ###          ##  #     preempted while holding the bus
```

Here's that exact scenario. Press run with `"inheritance": false` and watch `high` miss its deadline by 7 ticks. **Then flip inheritance on.** The low task gets boosted, finishes its critical section at tick 5, and high is done by 7. Try making medium's WCET 30: without inheritance, high's wait grows with it.

```playground
{
  "title": "Pathfinder in 40 ticks: priority inversion",
  "ticks": 40,
  "editable": true,
  "inheritance": false,
  "tasks": [
    { "name": "high", "priority": 3, "period": 40, "deadline": 8, "wcet": 2, "offset": 2, "lock": { "at": 0, "len": 1 } },
    { "name": "medium", "priority": 2, "period": 40, "wcet": 10, "offset": 3 },
    { "name": "low", "priority": 1, "period": 40, "wcet": 6, "offset": 0, "lock": { "at": 1, "len": 4 } }
  ]
}
```

### Priority inheritance

The rule: **while a task holds a mutex that a higher-priority task is waiting for, the holder runs at the waiter's priority.** When it unlocks, it drops back to its own priority, and the waiter gets the mutex (direct handoff).

```
tick           0    5    10   15   20
high   (3)       X..##                waits only for low's critical section
medium (2)            ##########
low (1->3)     #####            #     boosted to 3 while holding the bus
```

Medium can no longer cut in, because low now outranks it. The wait for high is bounded by *the length of low's critical section*. That leads to the second rule, which inheritance can't do for you:

> **Keep critical sections short.** Lock, touch the shared thing, unlock. Format the packet, compute the CRC and build the string *outside* the lock.

### Gotchas

- **A binary semaphore is not a mutex.** In FreeRTOS, `xSemaphoreCreateBinary` used as a lock has *no* priority inheritance. It's the Pathfinder bug waiting to happen. Use `xSemaphoreCreateMutex`.
- **Inheritance doesn't make long critical sections OK.** If low holds the bus for 5 ticks, high waits 5 ticks, boost or no boost. In this node's lab, that alone breaks the deadline.
- **Inheritance chains.** If low is itself waiting on another mutex held by an even lower task, the boost must propagate. Good kernels do this, and our simulator does too. Better still, avoid nested locks (next node: deadlock).
- **Never lock a mutex in an ISR.** An ISR can't own anything or wait for anything. Defer to a task, or protect very short data with a critical section.
- **The alternative is the priority ceiling.** Each mutex gets a "ceiling" priority, the highest of any task that uses it. Whoever locks it jumps straight to the ceiling. That's simpler to analyse and immune to chains and deadlock between ceiling mutexes. It's what OSEK/AUTOSAR use.

### In the wild

- **FreeRTOS:** `xSemaphoreCreateMutex()` (inheritance on), `xSemaphoreCreateRecursiveMutex()`. **Zephyr:** `k_mutex` (recursive, with inheritance). **POSIX/Linux:** `pthread_mutexattr_setprotocol(&a, PTHREAD_PRIO_INHERIT)`. The default is *no* inheritance, which surprises people every year.
- **PREEMPT_RT Linux** turns most kernel spinlocks into `rt_mutex`es with inheritance. That's a big part of how Linux gets real-time.
- **Watchdogs** did their job on Pathfinder: they turned a hang into a reset that the team could diagnose. Never treat a watchdog reset as "just a glitch".
- Mike Jones's December 1997 write-up, *"What really happened on Mars?"*, together with Glenn Reeves's reply, is still the best two pages ever written on this bug. Read them.

