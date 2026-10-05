### A console inside the device

Every serious piece of firmware has a back door for its developers: plug in a USB-serial cable, open a terminal, and the device answers. `led on`. `gpio get 7`. `reg read 0x0C`. When a board misbehaves on the bench, this little shell is how you poke at it without reflashing, and on a product line it's how test fixtures talk to every unit that comes off it.

You're going to write the core of one. Bytes arrive one at a time in a UART receive interrupt, and nothing may be lost or block while the main loop is busy. A line editor turns the bytes into lines: it echoes what you type, handles Backspace, copes with every terminal's idea of what Enter sends, and refuses lines that are too long instead of overflowing a buffer. A shell splits each line into words and looks the command up in a table. And the commands drive a peripheral's registers: set a bit, write a field, read a register by its offset, without disturbing the bits next door.

It all runs on your PC. The hardware sits behind a three-function HAL, and a simulator plays the UART and the device. So you can test everything with `make`, then (if you have a Nucleo or a Pico lying around) move the same files to a real board.

This is the Fundamentals section put to work: bit masks and fields on `volatile` registers, base-plus-offset register access, a lock-free single-producer single-consumer ring buffer fed by an ISR, overflow and overrun counting, a state machine with an `enum` and a `switch`, and fixed buffers with no `malloc` anywhere. The signatures and the tests are given; the code in between is yours.

**How to start:** copy the starter (the command is on this page), run `make` to check it builds, read the `README.md`, and start at milestone 1. `make test-m1` (and so on) tells you when each milestone is done, and `make run` lets you type at your shell as it grows.
