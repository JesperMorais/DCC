Between 1985 and 1987, the **Therac-25** radiation therapy machine gave six patients massive overdoses, and some of them died. One of the bugs: if an experienced operator edited the treatment settings *quickly*, within about 8 seconds, the software's setup task and its keyboard handler disagreed about what had been entered. The machine fired its high-power beam with no beam spreader in place.

It passed testing because testers typed slowly. The bug only existed in a timing window. That's a **race condition**: correctness depends on *when* things happen, not just *what* happens.

### The lost update

Here's the smallest race there is. An ISR counts pulses and `main()` also changes the counter:

```c
volatile uint32_t count;
void TIM_IRQHandler(void) { count++; }
void main_loop(void)      { count--; }   // consumed one
```

`count--` looks like one operation, but the CPU does it in three:

```
main:  LDR r0,[count]   ; r0 = 5
           ── interrupt! ISR: count = 6 ──
       SUB r0,r0,#1     ; r0 = 4
       STR r0,[count]   ; count = 4   ← the ISR's +1 is gone
```

The right answer is 5 and you got 4. Nothing crashed and nothing logged an error, so the count is just *quietly wrong*. It happens maybe once in a million runs, because the interrupt has to land in a two-instruction window. That's why races survive testing and show up in the field.

`volatile` doesn't help, as we saw in *Memory-mapped I/O*. It makes the loads and stores real, but it doesn't make the three of them one.

### The classic shapes

- **Lost update.** A read-modify-write (`x++`, `flags &= ~bit`, `x = x + n`) that is shared with an ISR or another task.
- **Torn read.** A value that takes two accesses to read. A 64-bit counter on a 32-bit CPU is two loads. If the ISR carries from the low word into the high word *between* them, you read a value that never existed.
- **Check-then-act.** `if (!busy) { busy = true; ... }`. Two contexts can both see `busy == false`.

### The fix: make the window atomic

On a single-core MCU, the bluntest tool is the most common one: **disable interrupts** around the shared access, so nothing can land in the window.

```c
__disable_irq();
count--;            // the ISR can't run in here
__enable_irq();     // a pending ISR runs right now, and nothing is lost
```

The interrupt isn't thrown away. The hardware holds it **pending**, and it fires the moment you re-enable. The only cost is *latency*: that interrupt runs a little late.

In an RTOS you'd use the kernel's version, `rtos_enter_critical()` / `rtos_exit_critical()` (FreeRTOS: `taskENTER_CRITICAL()`). It also stops the scheduler from switching tasks.

### Rule 1: keep it short

A critical section is a little time-freeze for the whole system. Everything inside it adds to the worst-case latency of **every** interrupt:

```c
__disable_irq();
uint32_t ev = pending; pending = 0;      // ✓ a few cycles
__enable_irq();
log_events(ev);                          // slow work outside
```

```c
__disable_irq();
log_events(pending); pending = 0;        // ✗ 2 ms with the UART deaf
__enable_irq();                          //   → ORE, lost bytes, missed ticks
```

Copy in, get out, and process afterwards.

### Rule 2: restore, don't blindly enable

What if your function is called from code that *already* disabled interrupts? A bare `__enable_irq()` at the end turns them back on in the middle of the caller's critical section, which creates the same race one level up. Save the previous state and restore it:

```c
bool was_enabled = dts_irq_enabled();    // CMSIS: __get_PRIMASK()
__disable_irq();
/* ... */
if (was_enabled) __enable_irq();
```

This is exactly why FreeRTOS has `taskENTER_CRITICAL_FROM_ISR()` returning a saved mask (its task-level `taskENTER_CRITICAL()` solves the same problem with a nesting counter instead), and why Zephyr's `irq_lock()` returns a key that you pass to `irq_unlock(key)`.

### Worked example: a torn 64-bit uptime

```c
volatile uint32_t lo, hi;                  // SysTick: if (++lo == 0) hi++;

uint64_t uptime(void) {                    // ✗ racy
    uint32_t l = lo;                       // 0xFFFFFFFF
    /* tick! lo = 0, hi = 1 */
    uint32_t h = hi;                       // 1
    return ((uint64_t)h << 32) | l;        // 0x1_FFFFFFFF: 49 days in the future
}
```

The fix is either a critical section around the two reads, or the lock-free trick: read `hi`, `lo`, `hi` again, and retry if `hi` changed.

### Gotchas

- **Hunting races with tests doesn't work** unless you *force* the interleaving. In the lab, the tests fire an interrupt exactly in the window. On hardware you'd use fault injection, or just reasoning.
- **Disabling interrupts doesn't protect you on multicore.** The other core keeps running. There you need spinlocks or atomics (`LDREX`/`STREX`, C11 `<stdatomic.h>`).
- **Single-writer designs need no lock.** That's what the ring buffer in the last node relied on. Often the best fix is to restructure until each variable has one writer.
- **Never block or delay inside a critical section.** The tick can't fire, so a delay never ends. The simulator catches `rtos_delay` in a critical section and fails the test.

### In the wild

- **Therac-25** is still the standard case study in medical-device software (IEC 62304 courses start with it).
- **The 2003 Northeast blackout:** a race in the GE XA/21 alarm system silently stalled alarms. Operators didn't know the grid was failing, and 55 million people lost power.
- **Linux** has `local_irq_save(flags)` / `local_irq_restore(flags)` (save and restore, just like Rule 2), plus `spin_lock_irqsave` for SMP.
- **Cortex-M** gives you `PRIMASK` (all interrupts), `BASEPRI` (only those below a priority, which FreeRTOS uses so urgent ISRs can still preempt) and `LDREX`/`STREX` for lock-free atomics.

In the lab you'll inherit firmware with three races in it. The tests fire real interrupts right in the middle of each read-modify-write until the bugs show, and then you'll fix them with short, nest-safe critical sections.
