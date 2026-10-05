### 1202: the alarm that saved Apollo 11

On 20 July 1969, during the last minutes of the descent to the Moon, the Lunar Module's guidance computer started flashing **1202** and **1201**: executive overflow. A switch left in the wrong position had the rendezvous radar flooding the CPU with work nobody needed. The computer was overloaded, and yet it didn't crash. Its executive, designed by Hal Laning and Margaret Hamilton's team, scheduled jobs **by priority**. When there wasn't enough CPU for everything, it restarted and dropped the low-priority jobs, while the high-priority guidance and display jobs kept getting the CPU. Mission control said "we're go on that alarm", and Armstrong landed.

That's the promise of priority scheduling. When the CPU is short, **you** decide in advance what gets dropped.

### Fixed-priority preemptive scheduling

Every task gets a fixed priority, and the rule is brutal and simple:

> **The highest-priority ready task is always the one running.**

If a more urgent task wakes up (its delay expires, an ISR gives it a semaphore), it **preempts** whatever is running, right away. The preempted task waits, mid-calculation, until nothing more urgent is ready. Equal priorities take turns (round-robin).

```
prio 3  motor   ##   ##   ##          motor wakes every 5 ticks and takes the CPU
prio 1  logger    ###  ###  ###       logger runs in the gaps, preempted mid-job
tick            0    5    10   15
```

This makes the scheduler predictable, and that's why you can *prove* deadlines. It also means a low-priority task gets only the leftovers.

### Response time: how long until a job is done?

A periodic task *i* has a period **Tᵢ** (it's released every Tᵢ ticks), a worst-case execution time **Cᵢ** and a deadline, here the end of its period. Its worst-case **response time** Rᵢ is the time from release until the job is done, in the worst case. The worst case is when every higher-priority task is released at the same moment (the *critical instant*). In that window, each higher-priority task *j* gets ⌈Rᵢ / Tⱼ⌉ jobs in ahead of you:

```
R = C_i + sum over higher-priority j of  ceil(R / T_j) * C_j
```

R appears on both sides, so you iterate: start with R = Cᵢ and recompute until it stops changing. If R ≤ deadline, the task is schedulable. If R overshoots the deadline, it isn't.

### Rate-monotonic: who gets the high priority?

**Not the most "important" task.** Priority is a scheduling tool, not a status symbol. Liu & Layland proved in 1973 that for periodic tasks with deadline = period, **rate-monotonic (RM)** assignment is optimal among fixed-priority schemes: *the shorter the period, the higher the priority*. If any fixed priority order meets all deadlines, RM does.

Why? A task with a short period has a short window to fit its work in. A task with a long period can absorb a few interruptions and still finish.

### Utilisation and the Liu & Layland bound

The CPU **utilisation** is U = Σ Cᵢ/Tᵢ. Over 100 % is impossible, period. Below that, Liu & Layland give a quick sufficient test for RM:

```
U  <=  n * (2^(1/n) - 1)

n = 1: 100 %   n = 2: 82.8 %   n = 3: 78.0 %   n = 4: 75.7 %   n -> inf: 69.3 %
```

If you're under the bound, RM is **guaranteed** to meet every deadline. Above it, *maybe*: the bound is sufficient, not necessary. Then you run the response-time analysis. A common lucky case is **harmonic periods** (each period divides the next, like 5, 10, 20). These are schedulable under RM right up to 100 %.

### Worked example: the drone camera

| task | T | C | C/T |
|---|---|---|---|
| motor | 5 | 2 | 40.0 % |
| camera | 15 | 4 | 26.7 % |
| logger | 30 | 6 | 20.0 % |

U = 86.7 %. That's above the 3-task bound of 78 %, so the quick test is inconclusive. But 5, 15, 30 are harmonic, so RM will make it. Let's check the logger, lowest under RM:

```
R = 6                       -> 6 + ceil(6/5)*2  + ceil(6/15)*4  = 14
R = 14                      -> 6 + ceil(14/5)*2 + ceil(14/15)*4 = 16
R = 16                      -> 6 + ceil(16/5)*2 + ceil(16/15)*4 = 22
R = 22                      -> 6 + 5*2 + 2*4                     = 24
R = 24                      -> 24, stable.  24 <= 30, schedulable.
```

Below, someone assigned priorities by importance: "the logger has our crash data, so make it top priority". **Press run, watch the motor miss, then fix the priorities so every deadline is met.** Then check that the logger's last tick is at 23, finishing at 24 just as the analysis predicted.

```playground
{
  "title": "Find the priority order that meets every deadline",
  "ticks": 60,
  "editable": true,
  "tasks": [
    { "name": "logger", "priority": 3, "period": 30, "wcet": 6 },
    { "name": "camera", "priority": 2, "period": 15, "wcet": 4 },
    { "name": "motor", "priority": 1, "period": 5, "wcet": 2 }
  ]
}
```

Try the other orders too. Camera above motor fails, and so does logger above camera. RM is the only order that works here.

### When no priority order works

Now two tasks with non-harmonic periods and U = 97 %:

```playground
{
  "title": "97 % utilisation: no priority order saves this",
  "ticks": 70,
  "editable": true,
  "tasks": [
    { "name": "servo", "priority": 2, "period": 5, "wcet": 2 },
    { "name": "vision", "priority": 1, "period": 7, "wcet": 4 }
  ]
}
```

Under RM, vision's response is 4 + 2·⌈R/5⌉: that's 8 > 7, a miss. Swap the priorities and servo misses instead. **Your challenge: change one number so that it fits.** Hint: cut vision's WCET to 3. U drops to 82.9 %, a hair *above* the 82.8 % bound, and it still meets every deadline. That shows the bound is pessimistic. (Earliest-deadline-first scheduling could run the original set at 97 %, but EDF is rare in small RTOSes, and it fails badly when overloaded.)

### Gotchas

- **Priority ≠ importance.** The crash logger is important. It still gets a low priority, because it has a long period and a lot of slack.
- **WCET means *worst case*.** It includes cache misses, the slow branch, and the time spent in ISRs that steal from every task. Measure, then add margin.
- **ISRs are above every task.** A 50 µs ISR at 10 kHz is 50 % of the CPU that your analysis has to include.
- **Blocking breaks the maths.** All of this assumes tasks never wait on each other. Once a high-priority task waits for a lock held by a low one, you need the next node (priority inversion).
- **Equal priorities** time-slice, so neither task's response time is what you'd calculate for it running alone.

### In the wild

- **FreeRTOS, Zephyr, VxWorks, QNX, ThreadX** and almost every RTOS use fixed-priority preemptive scheduling at their core. RM is the standard way to pick the numbers.
- **Automotive (AUTOSAR OS)** and **avionics (ARINC 653)** require timing analysis, often RTA plus measured WCET, before software can be certified.
- **Linux** has `SCHED_FIFO` and `SCHED_RR` for exactly this, and `SCHED_DEADLINE` for EDF.
- Apollo's lesson still applies: design for overload. Decide what gets shed **before** the radar switch is in the wrong position.
