# How we'd structure it

```
ISR ──rb_put──▶ ring ──rb_get──▶ line editor ──line──▶ shell_exec ──argv──▶ command ──▶ hal_dev() registers
(uart.c)        (ringbuf.c)      (line.c)              (shell.c)            (commands.c)
```

Only `uart.c` and `commands.c` touch registers, and only through `hal.h`, so the same five files run under the tests and on a board.

## Decision 1: one writer per variable

The ring needs no lock because nothing is written by both sides:

```c
uint32_t next = (head + 1) & (rb->size - 1);
if (next == rb->tail) { rb->dropped++; return false; }
rb->buf[head] = byte;
rb->head = next;        /* publish last */
```

The ISR owns `head` and `dropped`, the main loop owns `tail`. The byte is stored before `head` moves, and `rb_get` reads its byte before moving `tail`, so neither side ever sees a slot the other is still using. The storage is `volatile` too, because `volatile` only orders `volatile` accesses. This is enough on a single core with an ISR. With DMA or a second core you'd use C11 atomics with release/acquire ordering instead.

## Decision 2: count every loss, separately

Two kinds of loss mean different things. `dropped` means the main loop was too slow to empty the ring: make the ring bigger or the loop faster. An overrun means interrupts were masked too long for the UART's one-byte register: look for a long critical section. `stats` shows both, so a "sometimes a character goes missing" report comes with a cause. The policy for a full ring is to drop the newest byte, because overwriting the oldest would make the ISR write `tail` and break Decision 1.

## Decision 3: the line editor is a real state machine

The `\r\n` rule could be a `bool last_was_cr`, but then it's a hidden state that every other branch has to remember to reset. As a third enum value it's explicit: the switch forces you to say what happens in it, and "anything but `\n`: behave like collecting" is one fall-through. Overflow is a state for the same reason. Without it, the easy bug is to run the tail end of a too-long line as a command.

## Decision 4: the table is the single source of truth

`shell_commands[]` is `const`, so on a microcontroller it lives in flash, not RAM. `help` walks it, dispatch walks it, and adding a command is one row. `shell_exec` copies the line before splitting it, because `shell_split` is destructive by design (no allocation, just `\0`s and pointers) and the caller's string isn't ours to change.

## Decision 5: registers change one field at a time

Every write is a single read-modify-write that clears the field's mask and ORs in the new value, masked:

```c
dev->CTRL = (dev->CTRL & ~DEV_CTRL_BLINK_Msk) | ((rate << DEV_CTRL_BLINK_Pos) & DEV_CTRL_BLINK_Msk);
```

`reg` finds a register by adding the byte offset to the base address as an integer, then casting to `volatile uint32_t *`. Adding it to a `uint32_t *` would scale it by 4. 
One thing this design gets away with: only the main loop writes `CTRL` and `GPIO_OUT`. If an ISR wrote them too, each of those read-modify-writes would be a lost-update race, and it would need a short critical section (or a set/reset register like the STM32's `BSRR`).

## Where it could go next

Command history on the up arrow (`ESC [ A` is two more editor states), tab completion from the table, or a TX ring drained by a `TXE` interrupt so a long `help` doesn't stall the main loop.
