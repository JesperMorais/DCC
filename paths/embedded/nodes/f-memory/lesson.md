January 2004, sol 18 on Mars. The rover **Spirit** stopped talking sensibly to Earth and started rebooting, over and over, dozens of times. The cause wasn't radiation or a hardware fault. Spirit's flash filesystem had collected thousands of files since launch, and the table that tracked them kept growing in RAM until memory ran out. Each reboot tried to mount the filesystem, ran out of memory again, and rebooted.

JPL fixed it from more than 150 million km away. The lesson stuck: in a system that must never stop, **memory you allocate at runtime is memory you can run out of at runtime.**

### Where your variables actually live

A microcontroller has a little flash (code, constants) and a little RAM (everything that changes). The linker sorts your program into **sections**:

```
FLASH                          RAM
┌──────────────┐               ┌──────────────┐ high address
│ .text  code  │               │ stack  ↓     │ locals, return addresses
│ .rodata      │  const data   │              │
│ .data init   │──copied at──▶ │ heap   ↑     │ malloc (if you have one)
└──────────────┘    boot       │ .bss         │ zero-initialised globals
                               │ .data        │ initialised globals
                               └──────────────┘ low address
```

```c
const char banner[] = "v1.2";   // .rodata: flash, read-only
int threshold = 42;             // .data: RAM, initial value copied from flash at boot
static int samples[64];         // .bss: RAM, zeroed by the startup code
void f(void) { int x; }         // stack: exists only while f runs
```

The startup code that runs before `main()` copies `.data` from flash and zeroes `.bss`. Everything else is up to you.

### Why hard real-time code avoids `malloc`

`malloc` isn't evil, but it brings three problems you can't accept in a pacemaker or a brake controller:

1. **Non-deterministic timing.** `malloc` searches a free list. How long it takes depends on everything that happened before, so your worst-case execution time is "it depends".
2. **Fragmentation.** After hours of mixed-size allocs and frees, the heap looks like Swiss cheese. 6 KB are free in total, but no single hole holds 2 KB, so `malloc(2048)` fails. It works in a 10-minute test and fails after 3 weeks in the field.
3. **Failure has no good answer.** If `malloc` returns NULL inside a control loop, what do you do? There's no user to show an error dialog to.

That's why **MISRA C** bans the heap (Rule 21.3), and JPL's "Power of Ten" rule 3 says *no dynamic memory allocation after initialisation*.

### The workhorse: a fixed-block pool

When you do need "allocate a message buffer, free it later", use a **pool**: a static array of N blocks, **all the same size**.

```
blocks: [ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ]
free list: 3 → 0 → 6 → 7 → END      (head = 3)
```

- **alloc**: pop the head of the free list. That's O(1), every time.
- **free**: push the block back on. Also O(1).
- **Fragmentation is impossible.** Every hole is exactly one block, so any free block fits any request.
- **Exhaustion is bounded and testable.** You know at compile time that there are exactly 8. Size the pool for the worst case and *test* the empty-pool path.

A free list can be an array of indices, or it can live **inside the free blocks themselves**: the first bytes of each free block hold the "next" pointer, so the bookkeeping costs zero extra RAM. That second trick is how Zephyr's `k_mem_slab` and countless driver pools work, and FreeRTOS's `heap_4` keeps its (variable-size) free list inside the free blocks the same way.

### Worked example: catching a double free

A `free` that trusts its caller is a time bomb. If you free the same block twice, it's on the free list twice, so two later `alloc`s return **the same memory** to two owners. Then they corrupt each other, somewhere far from the bug.

Keep one "in use" bit per block and check it:

```c
bool pool_free(void *p) {
    size_t i = index_of(p);           // NULL, foreign or misaligned → reject
    if (i == BAD || !in_use[i]) return false;   // double free caught here
    in_use[i] = false;
    push_free(i);
    return true;
}
```

Turning a pointer into an index is just `(p - base) / BLOCK_SIZE`. It must also land **exactly** on a block boundary. A pointer into the middle of block 3 isn't block 3.

### Gotchas

- **Stack overflow is silent.** There's no MMU and no guard page by default, so a deep recursion or a 2 KB local array on a 1 KB stack just walks over `.bss`. Static analysis (`-fstack-usage`), stack painting and the MPU are your friends. The Toyota unintended-acceleration investigation found worst-case stack use that could overflow into OS data.
- **Alignment.** A block that holds a `uint64_t` or a `double` must be 8-byte aligned. A `uint8_t` array isn't guaranteed to be. Use `_Alignas(8)` (or a union) on your storage.
- **Pools hide leaks too.** If you never free, the pool runs dry. Keep a `free_count` you can log or show on a debug console, and alarm on a low-water mark.
- **"Static" doesn't mean small.** A 4 KB buffer in `.bss` costs 4 KB of RAM forever. Check the linker map file.

### In the wild

- **FreeRTOS** lets you choose `heap_1` (allocate only, never free), up to `heap_4`/`heap_5` (with coalescing), or `configSUPPORT_STATIC_ALLOCATION` so that every task and queue comes from your own static buffers.
- **Zephyr's `k_mem_slab`** is exactly the pool you're about to write, with an ISR-safe API.
- **Network stacks** (lwIP `pbuf` pools, Linux `skbuff` caches) use fixed-size buffer pools so that a packet flood can't fragment memory.
- **Avionics (DO-178C) and automotive (MISRA, AUTOSAR)** firmware is typically heap-free after init. That's an audited, written requirement.

In the lab you'll build the pool: static storage, O(1) alloc and free, a counter, a clean `NULL` when it runs dry, and a `free` that refuses double frees and foreign pointers.
