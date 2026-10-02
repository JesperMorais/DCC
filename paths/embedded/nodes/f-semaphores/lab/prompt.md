## Lost CAN frames

A battery-management system receives cell voltages over CAN. Every frame raises `can_rx_isr()`, and parsing a frame (`parse_frame()`) takes **2 ticks** of CPU. A `display` task from another team runs at **priority 2** and soaks up every spare tick.

The current code parses frames *inside the interrupt*. The simulator refuses to run it, and it's right to: on real hardware this freezes every task for the whole parse.

Fix it with **deferred interrupt processing**:

1. **`can_rx_isr()` only signals.** It gives `rx_sem` and returns. All the parsing happens in the `can_rx` task (the tests check that `can_rx` used the CPU ticks).
2. **Latency 0:** the handler must start parsing **on the same tick** as the interrupt, even while the display is busy. Choose its priority.
3. **No lost frames in a burst:** the controller can deliver up to `MAX_BURST` (16) frames back to back, or even on the same tick. The tests fire 5 frames in 5 ticks, and 12 frames on one tick. Every frame must be parsed exactly once, and the semaphore must be back at 0 afterwards.

Keep the names `can_rx_isr`, `can_start`, `rx_sem`, `frames_handled` and the task name `"can_rx"`. `parse_frame()` is given.

On the **Timeline**, look at the burst. With a binary semaphore, `can_rx` stops early. With counting, it works through the backlog.
