## The gimbal that twitches

A camera drone's gimbal controller runs three periodic tasks on one CPU. The priorities were picked in a meeting by "importance": telemetry goes to the customer's app, so it got the top priority. Now the gimbal twitches. Motor commutation is missing its deadlines.

| task | period | WCET | deadline |
|---|---|---|---|
| `motor` | 5 | 1 | end of its period |
| `imu` | 10 | 3 | end of its period |
| `telemetry` | 20 | 6 | end of its period |

All three are released together at tick 0. That's the worst case, the *critical instant*. The generic `periodic_task` is given and correct.

**Your job:**

1. Set `PRIO_MOTOR`, `PRIO_IMU` and `PRIO_TELEMETRY` (higher = more urgent) using **rate-monotonic** assignment, so that **no job of any task misses its deadline** over 1000 ticks.
2. Implement `double utilisation(const struct task_spec *set, size_t n)`, which returns U = Σ wcetᵢ / periodᵢ as a fraction (0.8 means 80 %). An empty set has U = 0.

The tests grade from the timeline: every job must get its full WCET of CPU inside its own period. They also check that the worst-case response times match the response-time analysis from the lesson: motor 1, imu 4, telemetry 15.

The gimbal set has U = 80 %. That's *above* the Liu & Layland bound for three tasks (78 %), so the quick test can't promise anything. The periods are harmonic (5 | 10 | 20), though, and the timeline will show that RM makes it.
