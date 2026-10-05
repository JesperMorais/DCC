# Writing skill-tree nodes

A **path** is a skill tree: `paths/<path>/path.json` lists its nodes, their sections (branches), their layout and their prerequisites. Each node's content lives in `paths/<path>/nodes/<node-id>/`.

```
nodes/<id>/
  lesson.md        the lesson (required). Markdown + ```playground blocks
  quiz.json        2–5 questions (required). Graded on the server
  lab/             the hands-on part (required for kind "lab" and "boss", absent for kind "lesson")
    meta.json      { "title", "profile": "embedded" | "linux", "estMinutes", "topics": [], "hints": [] }
    prompt.md      the task
    starter.c      compiles, fails the tests
    solution.c     passes, zero warnings
    tests.c        the tests (C harness, see challenges/README.md → C)
```

### path.json

| Field | Meaning |
|---|---|
| `language` | The labs' language (default `"c"`). Lab files use that language's extension, e.g. `starter.ts` for `"typescript"`. |
| `gate` | Optional. The node that ends the fundamentals and unlocks the branch choice. A path without one has no branches to pick. |
| `ranks` | Optional. `{ xp, title, blurb }[]` for the rank card. Default: the embedded ranks. |
| `nodes[].covers` | Daily-challenge `topics` this node teaches. When a daily on one of those topics goes badly (gave up, ≥2 hints, or over 2× `estMinutes`), the node is marked **needs practice**: it shows on the dashboard and opens early even if it's still locked. The mark clears once the quiz is passed and the lab is solved without hints after that daily, or after two clean dailies on the topic. It never blocks anything. |

A lab's `meta.json` may set `"level"` (1–6) to show it like a daily of that level. The default is 5, or 6 for a boss.

A node is **complete** once its quiz is passed (every answer right, with retries allowed) and, if it has a lab, the lab is solved. Stars: ★ for completing it, ★ for passing the quiz on the first try, ★ for solving the lab without hints (lesson-only nodes get the third star with the first). Completing nodes earns XP and unlocks the next nodes.

Validate with `npm run validate -- embedded.` (labs) and `npm run validate:paths` (lessons, quizzes and playground blocks).

## The teaching bar

- **Each node is a 15–25 minute session:** lesson about 6–10 min, quiz about 2 min, lab about 10 min. The learner should finish with one concept firmly in hand.
- **Make it fun and concrete.** Open with a real story or a real bug, for example Mars Pathfinder's priority inversion, the Toyota stack-overflow recalls, Therac-25 races, or "why your button bounces". Use plain language, short paragraphs, small code, and diagrams in ASCII where they help. End the lesson with **"In the wild"**: where this shows up in real products.
- **Lesson structure:** a hook (2–4 lines), then the idea, then a worked example, then the gotchas, then "In the wild". Use `###` headings. Teach; don't dump an API reference.
- **Branches teach the real API.** Map each concept back to the fundamentals ("this is the queue you built in Fundamentals, with FreeRTOS's name on it") and say what's different.
- **Quizzes test understanding, not trivia.** Use "predict the timeline", "which bug is this", "what breaks if…". Every question has an `explain` that teaches, including why the wrong options are wrong.
- **Labs** follow the challenge quality bar (challenges/README.md): realistic framing, the tests match the prompt exactly, hints escalate (nudge → approach → nearly the answer), and the reference solution is idiomatic and warning-free.

## Projects (kind "project")

One per section, unlocked by the section's boss, drawn beside it on the tree. The learner builds it **in their own editor**: the node page shows a command that copies `starter/` to `~/code/<folder>` and runs `npm install`. Milestones are ticked by the learner (honour system). A finished project is a badge: no XP, no stars, and it doesn't count towards the section. Its `covers` topics are used to suggest it on the dashboard when the dailies mark them as shaky.

```
nodes/<id>/project/
  project.json   { "title", "estHours", "folder", "testsGiven": true|false,
                   "milestones": [{ "id": "m1", "title", "body": "markdown", "hints": [3 strings] }] }
  brief.md       the story and the goal (≥150 words). What you're building and why, not how
  review.md      "How we'd structure it". Shown only after the project is done
  starter/       what the learner copies: package.json, tsconfig.json, README.md, src/, tests/
  solution/      the reference, copied over the starter by the validator. Never shown to the learner
```

**The balance: instructions vs. thinking.** A milestone says *what* must work and how you know it's done, never *how*. No "use reduce", no "create a Map". Scaffolding **fades** across the sections:

| | Fundamentals | Core | Generics |
|---|---|---|---|
| What to build | spelled out per milestone | spelled out per milestone | user stories only |
| Signatures | given (stubs that throw) | only the data types | designed by the learner |
| Tests | given, one file per milestone, testing the functions | given, one file per milestone, testing **behaviour** (run the CLI, check stdout/files) so any structure passes | written by the learner (`testsGiven: false`); the starter has one example test showing the syntax, and the milestone bodies list what the tests must cover |
| Files | given | suggested | the learner's |

**Hints** are exactly three per milestone, escalating, and **never code** (the validator rejects code-looking hints): 1. a question that gets them thinking ("what does the data look like after each step?"), 2. the concept and which node taught it, 3. a plan in words ("parse → filter → group → sort → print").

**Starter conventions.** ESM TypeScript, run with tsx and tested with Node's built-in test runner, so there are no other dependencies:

- `package.json`: `"type": "module"`, scripts `"start": "tsx src/main.ts"`, `"test": "node --import tsx --test \"tests/**/*.test.ts\""`, `"test:m1": "node --import tsx --test tests/m1.test.ts"` (one per milestone when tests are given), `"check": "tsc --noEmit"`; devDependencies `tsx`, `typescript`, `@types/node`.
- `tsconfig.json`: `strict`, `module`/`moduleResolution` `"nodenext"`, `target` `"es2022"`, `noEmit`, `allowImportingTsExtensions`, `types: ["node"]`, include `src` and `tests`. Imports use the `.ts` extension.
- Tests: `import { test } from "node:test"; import assert from "node:assert/strict";`. Milestone tests are named `tests/<milestone id>.test.ts`.
- `README.md`: how to run, test one milestone and type-check. Learners read this in their editor.

**C projects** (paths with `"language": "c"`): plain C17 built with `make`, no npm. The starter needs a `Makefile` with targets `build` (compiles everything with `-std=gnu17 -Wall -Wextra -Werror`; it may fetch a pinned dependency first, e.g. the FreeRTOS kernel), `test` (builds and runs every milestone's tests) and `test-m1`… per tested milestone, plus `README.md`. A last, optional milestone that can't be tested on the host, such as "port it to a real board", may have no tests; say so in its body. Tests print TAP-style lines, `ok N - name` / `not ok N - name`, and end with `# pass N` and `# fail N`. A small header in `tests/` (e.g. `check.h`) does that, so no test framework is needed. Set `"verify": "manual"` only when the toolchain can't run in validation (e.g. a full Zephyr workspace); the validator then checks the structure only and says so.

Validate with `npm run validate:projects`. It checks that the starter type-checks, that the given tests fail on the stubs, and that the solution type-checks and passes ≥2 tests per milestone.

## quiz.json

```json
[
  { "type": "single", "q": "Markdown question", "code": "optional C snippet", "options": ["A", "B", "C"], "answer": 1, "explain": "Markdown" },
  { "type": "multi", "q": "Select all that apply", "options": ["A", "B", "C", "D"], "answer": [0, 2], "explain": "…" },
  { "type": "order", "q": "Put these in order", "items": ["first", "second", "third"], "explain": "…" },
  { "type": "number", "q": "What's the CPU utilisation in %?", "answer": 75, "tolerance": 0.5, "unit": "%", "explain": "…" }
]
```

- `order`: write `items` in the **correct** order. The UI shuffles them.
- `answer` indices are 0-based.
- 2–5 questions per node, mixing types where it fits.

## Playground blocks (interactive scheduler)

Put a fenced block anywhere in `lesson.md`. It renders an interactive, editable scheduling simulation, with a timeline, deadline misses, CPU utilisation and the Liu & Layland RMS bound:

````
```playground
{
  "title": "Two periodic tasks",
  "ticks": 40,
  "editable": true,
  "inheritance": false,
  "tasks": [
    { "name": "sensor", "priority": 3, "period": 10, "wcet": 2 },
    { "name": "logger", "priority": 1, "period": 20, "wcet": 6, "offset": 0, "deadline": 20,
      "lock": { "at": 1, "len": 3 } }
  ]
}
```
````

- **Model:** fixed-priority preemptive, where a higher `priority` number is more urgent. Each task releases a job every `period` ticks starting at `offset`, and each job needs `wcet` ticks of CPU. `deadline` defaults to `period`.
- **Shared resource:** `lock` makes the job hold a shared resource R from execution tick `at` for `len` ticks. Any task with a `lock` competes for the same single R, and `inheritance` turns on priority inheritance. That's all you need to show priority inversion live.
- **What learners can do:** with `editable: true`, they can change the numbers and see what happens. That's the fun part, so pose a challenge in the text ("can you make the logger miss its deadline?").

## Labs: the simulated hardware and kernel

Labs are C, compiled with `gcc -std=gnu17 -Wall -Wextra`, AddressSanitizer + UBSan and a leak check, exactly like the C track. Each `TEST` runs in its own process, so every test starts with a fresh kernel and fresh "hardware". Labs never define `main()`.

### `"profile": "embedded"`

The headers below are available. Include what you use in **both** the starter/solution and the tests.

| Header | What it is |
|---|---|
| `mcu.h` | Simulated STM32-style peripherals. `GPIOA..C` (`MODER`, `IDR`, `ODR`, `BSRR`) and `USART1` (`SR`, `DR`, `BRR`, `CR1` plus bit macros). The learner may define `USART1_IRQHandler` / `SysTick_Handler`. `__disable_irq()` / `__enable_irq()` really hold interrupts pending. Test side: `sim_uart_receive(byte)`, `sim_systick(n)`, `sim_gpio_set_input(port, pin, high)`, `sim_gpio_apply_bsrr(port)`. |
| `rtos.h` | The neutral RTOS used in Fundamentals. Tasks (`rtos_task_create(name, fn, arg, prio)`, where a higher number is more urgent), `rtos_delay`, `rtos_delay_until`, `rtos_yield`, `rtos_busy(ticks)` (simulated CPU work), `rtos_now`, critical sections, semaphores (`rtos_sem_create(initial, max)`, take/give, give is ISR-safe), mutexes (`rtos_mutex_create(priority_inheritance)`), queues (`rtos_queue_create(len, item_size)`, send/receive with timeouts), notifications, and timers (callback runs in ISR context). Timeouts are in ticks, with `RTOS_NO_WAIT` / `RTOS_WAIT_FOREVER`. |
| `sim.h` | For tests (and occasionally learners). `sim_run(ticks)` runs virtual time. `sim_irq_at(tick, name, isr)` and `sim_irq_every(first, period, name, isr)` raise interrupts. Introspection: `sim_ran_ticks(name)`, `sim_running_at(tick)`, `sim_first_run_at_or_after(name, tick)`, `sim_context_switches()`, `sim_deadlocked()`, `sim_task_finished(name)`, `sim_max_priority_seen(name)`, `sim_mark(label)`. Also `sim_set_time_slicing(bool)`. |
| `FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`, `timers.h` | Real FreeRTOS names and semantics (subset). `xTaskCreate`, `vTaskDelay`, `vTaskDelayUntil`/`xTaskDelayUntil`, `xTaskGetTickCount`, `vTaskPrioritySet`, `vTaskDelete(NULL)`, `xQueueCreate`/`Send`/`Receive`/`SendFromISR`, `xSemaphoreCreateBinary` (starts empty)/`Counting`/`Mutex` (with priority inheritance), `Take`/`Give`/`GiveFromISR`, `portYIELD_FROM_ISR`, `xTaskNotifyGive`/`vTaskNotifyGiveFromISR`/`ulTaskNotifyTake`, `xTimerCreate`/`Start`/`Stop` (callbacks run in the "Tmr Svc" daemon task at prio 6), `pdMS_TO_TICKS`, `portMAX_DELAY`, `configMAX_PRIORITIES` (8), `taskENTER_CRITICAL`. `vSimulateWork(ticks)` stands in for CPU work. **Important FreeRTOS semantic:** a task woken from an ISR only runs before the next tick if the ISR calls `portYIELD_FROM_ISR(pdTRUE)`. Without it, the response is one tick late. |
| `zephyr/kernel.h` | Real Zephyr names and semantics (subset). `K_THREAD_DEFINE` (threads start when the kernel boots, i.e. on the first `sim_run`), `k_thread_create`, `k_msleep`/`k_sleep(K_MSEC(n))`, `k_yield`, `k_uptime_get`, `k_busy_wait(us)` (burns CPU), `K_SEM_DEFINE`/`k_sem_*` (take returns 0, -EBUSY or -EAGAIN), `K_MUTEX_DEFINE`/`k_mutex_*` (recursive, with priority inheritance), `K_MSGQ_DEFINE`/`k_msgq_put`/`get`, `K_WORK_DEFINE`/`k_work_submit` (the system workqueue thread runs at prio -1, cooperative), `K_TIMER_DEFINE`/`k_timer_start` (expiry runs in ISR context), `CONTAINER_OF`, `printk`. **Zephyr priorities:** a LOWER number is MORE urgent, and **negative priorities are cooperative**: they are never preempted by other threads until they sleep, wait or yield. |

**How simulated time works**
- 1 tick = 1 ms.
- Code between kernel calls takes zero time. Only `rtos_busy` / `vSimulateWork` / `k_busy_wait`, delays and waiting make time pass.
- Interrupts fire at tick boundaries. Equal priorities round-robin one tick at a time (neutral RTOS, FreeRTOS), or in 20 ms slices under Zephyr (`CONFIG_TIMESLICE_SIZE=20`, the default).
- Zephyr relative timeouts wait one extra tick, as on real Zephyr: `k_msleep(5)` takes 6 ticks, and `k_sem_take(&s, K_MSEC(10))` times out after 11. Use `K_TIMEOUT_ABS_MS(t)` or a `k_timer` for drift-free periodic work: they land exactly on the grid.
- Queues follow FreeRTOS: a send to a waiting receiver goes into the queue and wakes it, so a queue of length N holds N items in a burst. Zephyr's `k_msgq` copies straight into a waiting receiver, as real Zephyr does, so a burst gets one extra message through.
- FreeRTOS `…FromISR` calls set `*pxHigherPriorityTaskWoken` only when the woken task outranks the interrupted one.
- A task paused mid-`rtos_busy` when `sim_run()` returns resumes in the next `sim_run()`, so tests may split runs freely.
- Tests drive time with `sim_run()` and then assert on state, counters or timing, for example `sim_first_run_at_or_after("handler", 10) == 10`.

**What the simulator catches** (the test fails with a teaching message):
- blocking calls from an ISR
- delays inside critical sections
- unlocking a mutex you don't own
- re-locking a non-recursive mutex
- deadlocks (`sim_deadlocked()`)
- a task spinning forever without a kernel call, which hits the 1.5 s real-time limit and is reported as an infinite loop

**Timeline:** every test that uses the kernel produces a timeline (which task ran on each tick, plus interrupts, locks and priority changes). The learner sees it in a "Timeline" tab. Design labs so the timeline tells the story: name tasks meaningfully, and use `sim_mark("...")` for key moments.

Kernel objects belong to the simulator and are freed after each test, so the leak check only applies to the learner's own `malloc`s.

### `"profile": "linux"`

These labs use real Linux userspace APIs on the host (`-D_GNU_SOURCE -pthread -lrt`): files, `poll`/`epoll`, `eventfd`, `timerfd`, pipes, `pthread_*` (including `PTHREAD_PRIO_INHERIT`), signals and `clock_*`.

- **No real hardware and no root.** Simulate sysfs, devices and sensors with files, pipes and fixture directories that the tests create in the current working directory (a private temp dir per run).
- **Keep timing-based assertions robust**, because the host is not real-time. Prefer injected clocks or pure functions, for example "compute the next absolute deadline". If you must test real timing, use generous tolerances and short runs (each test has 1.5 s).
- **Kernel-driver topics:** model the driver interface in user space (a `struct file_operations`-style table of function pointers, a fake `struct file`, `copy_to_user` stand-ins). Explain in the lesson what's different in the real kernel.
