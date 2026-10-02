## The motor that stuttered every time it logged

A battery-powered pump runs Zephyr on an nRF52840. Three threads share the CPU, all created with `K_THREAD_DEFINE`:

| Thread | What it does | Timing |
|---|---|---|
| `control` | reads the encoder and updates the motor PWM, 1 ms of work | released every **10 ms** (0, 10, 20, ...) |
| `radio` | a BLE-style TX burst: preamble, payload, CRC, 1 ms each | wakes at 9, 59, 109, ... |
| `logger` | erases and programs one flash page, **25 ms** of CPU | then sleeps 5 ms, forever |

Since the last firmware update the pump *stutters*. The previous engineer came from FreeRTOS and picked the priorities you see in the starter. **Fix the three `#define`s. Don't change the thread bodies.**

Requirements (each is a test):

1. **Control is never more than 3 ms late.** Every release at `10·k` must start running by `10·k + 3`.
2. **Control is exactly on time when nothing else is due.** At 20, 30 and 40 it starts on the dot.
3. **The radio burst is atomic.** At 9–11, 59–61, 109–111 and 159–161 the CPU runs `radio` and nothing else. A burst that gets split corrupts the packet.
4. **The logger still makes progress**, with at least 350 of 500 ms of CPU. You can't fix this by starving it.
5. **Say what you mean:** `control` has a *more urgent* (lower) priority number than `logger`, and `logger` is **preemptible** (>= 0).

Remember the Zephyr rules: **a lower number is more urgent**, and **negative priorities are cooperative**. Once a cooperative thread is running, nobody else gets the CPU until it sleeps, waits or yields.

Open the **Timeline** with the starter and look at tick 10. That's the bug, in one picture.
