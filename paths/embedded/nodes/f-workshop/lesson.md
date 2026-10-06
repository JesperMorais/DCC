So far every lab ran in the browser. Real firmware lives in a folder of `.c` and `.h` files, a Makefile and a terminal. This workshop gets you there, with the UART shell project as the example. Nothing here is timed. Type each command and look at what comes back. Every output below is real, made by breaking the project on purpose.

### 1. Check your tools

Use Linux, or WSL2 on Windows (`wsl --install` in PowerShell, then work inside Ubuntu). On Ubuntu or Debian:

```sh
sudo apt install build-essential gdb git
gcc --version; make --version; gdb --version
```

Each prints a version line; `command not found` means that tool is missing. On macOS, `xcode-select --install` gives you `clang` (answering to `gcc`), `make` and `git`. The project builds there; use `lldb` where this page says gdb.

### 2. Terminal basics and the copy command

`pwd` says where you are, `ls` what's here, `cd src` goes in, `cd ..` goes up. Tab completes names, the up arrow repeats a command, Ctrl-C stops a running program. The project page gives you a command like this:

```sh
mkdir -p ~/code && cp -r ".../f-project/project/starter" ~/code/uart-shell && cd ~/code/uart-shell
```

`mkdir -p` makes `~/code`, `cp -r` copies the starter folder and everything in it, and `&&` runs the next command only if the previous one worked. Then open the folder in your editor.

### 3. Anatomy of a Makefile

`make` reads `Makefile` and rebuilds only what's out of date. Its building block is a rule:

```make
build/tests: $(CORE) $(TESTS) $(HEADERS)
	$(CC) $(CFLAGS) $(SANITIZE) -Itests -o $@ $(CORE) $(TESTS)
```

The **target** comes before the colon, its **prerequisites** after it. If any prerequisite is newer than the target, the recipe runs. The recipe line must start with a **tab**: spaces give `missing separator`. `CC = gcc` sets a **variable**, `$(CC)` uses it, `$@` means "this target". `.PHONY: test clean` marks targets that aren't files, so `make test` always runs.

The flags: `-Wall -Wextra` turn on warnings, `-g` keeps debug info for gdb, and `-Werror` turns every warning into an error. That's on purpose: in C, a warning is usually a bug that hasn't happened yet.

```
src/ringbuf.c: In function ‘rb_count’:
src/ringbuf.c:34:14: error: unused variable ‘mask’ [-Werror=unused-variable]
   34 |     uint32_t mask = rb->size - 1;
      |              ^~~~
cc1: all warnings being treated as errors
make: *** [Makefile:24: build/uart-shell] Error 1
```

Read the first `error:` line. `cc1: all warnings being treated as errors` is gcc explaining why a warning stopped the build, and `make: ***` is make saying a recipe failed. Neither is a second problem.

### 4. Many files: headers, `static` and the linker

Each `.c` file is compiled on its own; then the **linker** joins them and connects every call to its function body. A **header** holds declarations (types, `#define`s, prototypes), so every file that includes it agrees on them. The `.c` file holds the definitions.

```c
#ifndef RINGBUF_H          /* include guard: a second #include of this file is skipped */
#define RINGBUF_H
bool rb_put(ringbuf_t *rb, uint8_t byte);   /* the promise; the body is in ringbuf.c */
#endif
```

`static` on a function in a `.c` file makes it private to that file: helpers should be static, or two files with an `is_space` collide (``multiple definition of `is_space'``). `extern const shell_cmd_t shell_commands[];` in a header declares an array that one `.c` file defines.

Two errors look alike but come from different stages. The **compiler** met a call with no declaration in sight (a missing `#include "out.h"`, or a typo):

```
src/line.c:19:5: error: implicit declaration of function ‘out_puts’ [-Wimplicit-function-declaration]
```

The **linker** found the declaration but no linked file defines the body. Usually the new `.c` file isn't in the Makefile's list:

```
/usr/bin/x86_64-linux-gnu-ld.bfd: /home/you/code/uart-shell/src/shell.c:16:(.text+0x25): undefined reference to `is_space'
collect2: error: ld returned 1 exit status
```

### 5. Run the tests and read them

`make test-m1` runs one milestone. On the untouched starter:

```
#   tests/test_m1.c:15: CHECK(rb_init(&rb, s, 2)) failed
#   tests/test_m1.c:16: CHECK(rb_init(&rb, s, 64)) failed
not ok 1 - m1: rb_init accepts only powers of two >= 2
...
1..11
# pass 2
# fail 9
```

This is TAP: one `ok` or `not ok` line per test, with its `#` findings printed *above* it. Open `tests/test_m1.c` at line 15: the input is right there. Two tests pass on stubs that do nothing; that's normal. To rerun one test, give any part of its name: `make test T=powers`.

### 6. When a sanitizer speaks

The tests run with AddressSanitizer and UBSan. Here `rb_put` wrapped one slot late:

```
==2890712==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x730d04af0084 ...
WRITE of size 1 at 0x730d04af0084 thread T0
    #0 0x5dfd747ecb28 in rb_put src/ringbuf.c:21
    #1 0x5dfd747f1160 in ring_wraps_around tests/test_m1.c:64
  ...
    [128, 132) 's' (line 58) <== Memory access at offset 132 overflows this variable
#   exited with status 1: see the sanitizer report above
not ok 4 - m1: the indices wrap around
```

Read *what* (a 1-byte write past a stack array), *where* (frame `#0` is your line) and *whose memory* (`s`, 4 bytes, written one past its end). Skip the shadow-byte table. UBSan says it in one line. This `parse_u32` collected digits in an `int`:

```
src/commands.c:76:28: runtime error: signed integer overflow: 429496729 * 10 cannot be represented in type 'int'
```

### 7. Look at the values: fprintf and gdb

The quickest probe is `fprintf(stderr, "head=%u tail=%u\n", rb->head, rb->tail);`. The tests check the simulated UART, never stderr. For a closer look, `make debug T="m2 backspace on an empty"` runs `gdb --args ./build/tests ...`. Here backspace forgot to check for an empty line:

```
(gdb) break line_feed
(gdb) run
Thread 2.1 "tests" hit Breakpoint 1, line_feed (l=..., c=127 '\177') at src/line.c:22
(gdb) bt
#0  line_feed (l=..., c=127 '\177') at src/line.c:22
#1  feed (...) at tests/test_m2.c:10
#2  backspace_on_empty_line () at tests/test_m2.c:75
(gdb) print l->len
$2 = 0
(gdb) finish
Value returned is $3 = LINE_NONE
(gdb) print l->len
$4 = 4294967295
```

Zero minus one, unsigned. `next` runs one line, `step` goes into a call, `finish` runs to the end of this function, `continue` to the next breakpoint, `quit` leaves.

### 8. git, and the smallest next step

```sh
git init
git add -A && git commit -m "starter builds"
```

Commit each time something goes green. `git diff` shows what changed; `git restore src/line.c` takes a file back to the last commit. The starter's `.gitignore` keeps `build/` out.

Then don't write all of milestone 1. Make `rb_init` reject the bad sizes, run `make test T=powers`, see `ok 1`, commit. Then the next test. Getting stuck along the way is the work, not a detour from it.

### In the wild

- **Vendor IDEs** like STM32CubeIDE run make (or CMake) and gcc underneath. `arm-none-eabi-gcc` takes the same flags, and the Linux kernel has `CONFIG_WERROR`.
- **Host testing** is standard practice: Zephyr runs its test suites on `native_sim`, and James Grenning's *Test-Driven Development for Embedded C* is built on this workflow.
- **gdb on a real board** is the same commands: `arm-none-eabi-gdb` connects to a probe through OpenOCD (`target remote :3333`), then `break`, `bt`, `print`.
