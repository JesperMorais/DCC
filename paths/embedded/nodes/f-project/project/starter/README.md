# uart-shell

The firmware core of a device's debug shell: bytes arrive over a UART, a
line editor turns them into lines, and a small command table runs them against
the device's registers. It's plain portable C. The only file that knows it runs
on a PC is `src/hal_host.c`. The milestones (in the app) say what each part must do.

## Build, run, test

```sh
make                         # builds build/uart-shell and build/tests
make run                     # type at the shell. Ctrl-C or Ctrl-D quits
make test-m1                 # one milestone's tests (m1 to m4)
make test                    # all of them
make test T=backspace        # only the tests whose name contains "backspace"
make debug T="m2 backspace"  # run that test under gdb (see "When you're stuck")
```

`make run` puts your terminal in raw mode, so every key you press goes to the
"UART" as one byte, exactly like a serial terminal connected to a board: Enter
arrives as `\r`, Backspace as `0x7F`, and nothing appears on screen unless your
code echoes it. You can also pipe a script in:

```sh
printf 'help\nled on\nreg read 0\n' | ./build/uart-shell
```

The tests are in `tests/`, one file per milestone. Each test runs in its own
process with AddressSanitizer and UBSan, so a crash, an out-of-bounds write or
an endless loop fails that one test with a message instead of taking the whole
run down. When a test fails, read it: the exact expected output is in there.
If your gcc has no sanitizers, use `make SANITIZE= test`.

Later milestones build on earlier ones: the m3 tests type into the UART, so
they need your m1 and m2 code to work.

The build uses `-Werror`, so every warning stops it. You'll see the warning
marked `error: ... [-Werror=unused-variable]` and then the line
`cc1: all warnings being treated as errors`. That last line is not a second
problem, just gcc saying why a warning broke the build. Fix the warning above
it. They're on purpose: in C, most warnings are a bug that hasn't happened yet.

## When you're stuck

The node *Workshop: C on your own machine* walks through each of these tools
with real output from this project.

1. **Read the first failing test only.** Each test prints its findings and
   *then* its verdict, so the `#` lines belong to the `not ok` line below them:

   ```
   #   tests/test_m2.c:128: FEED(&l, "\r") is LINE_NONE (0), expected LINE_READY (1)
   #   tests/test_m2.c:130: sim_tx() is not what we expected
   #     got       ""
   #     expected  "\r\n"
   not ok 11 - m2: Enter on an empty line gives an empty line
   ```

   `file:line` is the check that failed: open it, the test's input is right
   there. `X is A, expected B` means your code returned A. Strings show `got`
   and `expected` with `\r`, `\n` and `\b` made visible, and enum values are
   printed by name. Fix that one, rerun, and only then look at the next. Later
   failures are often the same bug again.
   If the line says `exited with status 1: see the sanitizer report above`,
   scroll up: the report's first `#0 ... in function file.c:line` (ASan) or
   `file.c:line: runtime error:` (UBSan) is where your code went wrong.
   `timed out after 2 s` almost always means a loop that never ends.
2. **Run just that milestone** (`make test-m2`), or one test by name:
   `make test T="empty line"` or `./build/tests m2 backspace`. Any part of the
   name after `not ok N -` works.
3. **Look at the value.** Put `fprintf(stderr, "len=%u state=%d\n", l->len, l->state);`
   where you're unsure (with `#include <stdio.h>` at the top). stderr shows up in the test output but the tests never
   check it. (Don't use `out_printf` for this: that goes to the simulated UART
   and the tests do check it.) For a closer look, `make debug T="m2 backspace"`
   starts gdb on that test: `break line_feed`, `run`, then `bt` (who called
   me?), `print *l`, `next`, `finish`. Pick a name that matches one test.
   Variables shown as `<optimized out>`? `make clean && make OPT=-O0`.
4. **Make the step smaller.** One function, one test, one line at a time. A
   ring that can't `put` one byte and `get` it back won't survive the ISR test.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to
   reread.
6. **Commit when green.** `git init` once (there's a `.gitignore` for `build/`),
   then `git add -A && git commit -m "m2 green"` after each milestone, so you can
   always get back to a working version. `git diff` shows what you changed since.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.

## Files

| File | What it is | Yours? |
|---|---|---|
| `src/periph.h` | The register blocks, their offsets and bits | given |
| `src/hal.h` | The three functions the core uses to reach hardware | given |
| `src/hal_host.c`, `src/sim.h` | The PC implementation of the HAL, and the simulator hooks the tests use | given |
| `src/out.h`, `src/out.c` | `out_putc`, `out_puts`, `out_printf` over the UART | given |
| `src/main.c` | The PC "board": each key press becomes an interrupt, then the main loop runs | given |
| `src/ringbuf.c` | The SPSC ring buffer | m1 |
| `src/uart.c` | The RX interrupt handler and the main-loop side of the driver | m1 |
| `src/line.c` | The line editor state machine | m2 |
| `src/shell.c` | Splitting, dispatch, the main loop | m3 |
| `src/commands.c` | The command table and the commands | m3, m4 |

Every function you write already exists as a stub marked `TODO(mN)`, and the
header next to it says what it must do. Add `static` helpers wherever you like.
Don't change the headers: the tests are written against them.

## The simulated hardware

**UART** (`hal_uart()`): `SR` and `DR`, laid out and numbered like an STM32F4
USART. When a byte arrives, the simulator puts it in `DR`, sets `RXNE` in `SR`
and calls your `uart_rx_isr()`, just as the NVIC would. If a byte arrives while
`RXNE` is still set (interrupts were masked too long), it's lost and `ORE` is
set instead. `TXE` is always set and belongs to the transmit side.

**The device** (`hal_dev()`):

| Offset | Register | Access | Contents |
|---|---|---|---|
| `0x00` | `CTRL` | rw | bit 0 `LED`, bits 4..7 `BLINK` rate (0 = steady, 1..15 Hz); other bits reserved |
| `0x04` | `GPIO_OUT` | rw | bits 0..15: the level we drive on pins 0..15 |
| `0x08` | `GPIO_IN` | ro | bits 0..15: the level the outside world drives |
| `0x0C` | `ID` | ro | always `0xC0DE0042` |

Output goes through `hal_putc`. On the PC it's captured for the tests and,
under `make run`, copied to your terminal. Lines end in `\r\n`, as they do on
every serial console.

## Milestone 5 (optional): run it on a real board

Nothing tests this one. It's here because the whole point of the HAL is that
`ringbuf.c`, `uart.c`, `line.c`, `shell.c` and `commands.c` move to a
microcontroller unchanged. You replace `hal_host.c`, `main.c` and `sim.h`.

**STM32 Nucleo-F401RE** (any STM32F4 Nucleo is close). Its USART2 is wired to
the ST-LINK's USB virtual COM port, and our `uart_regs_t` matches the first two
registers of an F4 USART, so the core's ISR works on the real thing.

1. Make an empty bare-metal project for the board (STM32CubeIDE, or your own
   startup file, linker script and CMSIS headers) and add the core files.
2. Write `hal_board.c`:
   - `hal_uart()` returns `(uart_regs_t *)USART2`, which is at `0x40004400`.
   - `hal_putc()` waits for `TXE` in `USART2->SR`, then writes `DR`.
   - `hal_dev()` returns a `dev_regs_t` kept in RAM (no chip has our exact
     device). Your main loop mirrors it to the hardware: `LED` and `BLINK`
     drive LD2 on PA5, using SysTick for the blink, and a few real input pins
     are copied into `GPIO_IN`.
3. Bring-up in `main()`: enable the GPIOA clock (`RCC->AHB1ENR` bit 0) and the
   USART2 clock (`RCC->APB1ENR` bit 17). Put PA2/PA3 in alternate function 7 and
   PA5 in output mode. Set `BRR` to 139 (16 MHz reset clock / 115200 baud), then
   set `UE`, `TE`, `RE` and `RXNEIE` in `CR1`. Finally, enable `USART2_IRQn` (38)
   in the NVIC.
4. Define `void USART2_IRQHandler(void) { uart_rx_isr(); }`, then call
   `shell_init()` once and `shell_poll()` forever.
5. Connect with `picocom -b 115200 /dev/ttyACM0` (Ctrl-A Ctrl-X quits).

On the F4, reading `DR` already clears `RXNE`, and the `SR` then `DR` read
sequence clears `ORE`. The write of zeros that the simulator needs is harmless
there, but it isn't needed, and a read-modify-write of `SR` can clear a `TC`
flag that gets set in between. A real port drops it.

**Raspberry Pi Pico.** The RP2040 has a different UART (an Arm PL011 with a
FIFO, on GP0/GP1), so this is where the HAL boundary earns its keep. Write a Pico
version of `uart.c` whose interrupt handler uses the pico-sdk
(`uart_is_readable` / `uart_getc` in a loop, because one interrupt can mean
several bytes in the FIFO) and puts each byte in the same kind of ring. Keep `line.c`, `shell.c` and `commands.c` as they are. The LED is
GP25 on a plain Pico. Watch out: the pico-sdk already has a function called
`uart_init`, so rename ours.
