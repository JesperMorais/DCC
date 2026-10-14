# Writing challenges

Challenges live in `challenges/<language>/<slug>/`, and the id is `<language>/<slug>`. Languages: `typescript`, `python`, `c`.

| File | What it holds |
|---|---|
| `meta.json` | `title`, `level` (1–6), `rating`, `topics[]`, `estMinutes`, `hints[]`, and for TypeScript only `mode` (`"runtime"` or `"types"`) |
| `prompt.md` | The task, in Markdown. Short and concrete, with 2–3 input → output examples. |
| `learn.md` | The **Concept** tab: a mini-lesson (60–200 words plus a small code example) on the idea being practised. It teaches the idea but **doesn't give away the solution**. |
| `starter.<ext>` | What the learner starts with. It must compile/parse on its own and must **fail** the tests. |
| `solution.<ext>` | The reference solution. It must pass and be **warning-free**. It's shown after the learner solves the challenge or gives up. |
| `tests.<ext>` | The tests, which the learner can see. Aim for 4–7, including edge cases (empty input, zero, negatives, boundaries). |

Check your work with `npm run validate` (or `npm run validate -- python/l3` to filter). It must report 0 problems.

## Quality bar (applies to every language)

- **About 10 minutes** for someone at that level (`estMinutes` 5–12). These are daily katas, not projects.
- **Realistic framing** where you can (a shopping cart, log lines, sensor readings…), and **one clear concept** per challenge.
- **Hints:** 2–3 of them, escalating. The first is a nudge, the second gives the approach, the last is almost the answer.
- **Tests match the prompt exactly.** Every rule in the prompt has a test, and every test is explained by the prompt.
- **Idiomatic reference solutions**, the kind a senior engineer would approve in code review.
- Keep function and type names unique within a language's bank.

## Levels & rating bands (same bands for every language, different names)

| Level | Band | TypeScript | Python | C |
|---|---|---|---|---|
| 1 | 700–850 | First steps | First steps | First steps |
| 2 | 850–1000 | Building blocks | Building blocks | Loops & arrays |
| 3 | 1000–1150 | Shaping data | Data structures | Strings & chars |
| 4 | 1150–1300 | Generics | Functions & classes | Pointers |
| 5 | 1300–1450 | Advanced patterns | Pythonic patterns | Memory & structs |
| 6 | 1450–1650 | Type wizardry | Expert Python | Systems craft |

Level 1 in every language assumes the learner has **never coded**, so `learn.md` explains what a variable, function or type is.

---

## TypeScript (`.ts`)

- **No `import`/`export`.** The learner's file and `tests.ts` are global scripts that share one scope.
- Compiled with `strict: true`, lib ES2023, no DOM. Globals: `console`, `setTimeout`, `clearTimeout`, `queueMicrotask`, `structuredClone`.
- Tests are type-checked together with the learner's code, so a wrong signature is a compile error.
- Tests: `test("name", () => { expect(x).toBe(y); })`. Matchers: `toBe`, `toEqual`, `toBeCloseTo`, `toBeTruthy/Falsy`, `toBeUndefined/Null`, `toBeGreaterThan/LessThan`, `toHaveLength`, `toContain`, `toBeInstanceOf`, `toThrow(msg?)`, and `.not.<matcher>`.
- Type-level challenges (`"mode": "types"`) are judged by the compiler alone: `Expect<Equal<A, B>>`, `ExpectFalse<…>`, `// @ts-expect-error`.

## Python (`.py`)

- Runs on **CPython 3.12+** (the dev machine has 3.14), stdlib only. The learner's code runs as a module, and the tests run in the **same namespace**, so they call the learner's functions and classes directly. **Don't import the learner's code.**
- **Tests are pytest-style:** module-level `def test_something():` functions using plain `assert`. The harness rewrites asserts so a failure shows expected vs received. The display name is the function name with underscores turned into spaces, or the first line of its docstring.
  - `with raises(ValueError, match="regex"):` checks that an exception is raised.
  - `assert area(2) == approx(12.566, rel=1e-3)` compares floats.
  - Both `raises` and `approx` are built in. Don't import pytest.
  - `async def test_…()` works (it's run with `asyncio.run`).
  - Each test has a **1.5 s** time limit. After an infinite loop, the remaining tests are skipped.
- **Type hints are part of the lesson.** Starters and solutions are fully annotated. The learner's file is checked with `mypy --strict`, and findings are shown as **warnings** (they don't block a solve). **Reference solutions must be mypy-clean.** Use modern syntax: `list[int]`, `dict[str, int]`, `X | None`.
- **Idiomatic Python:** comprehensions, `enumerate`, `zip`, f-strings, `dataclasses`, `collections`, `itertools`, `functools`, and EAFP where it fits. PEP 8 naming.
- Starters usually `return` a placeholder (`0`, `""`, `[]`, `None`) or `raise NotImplementedError`.

## C (`.c`)

- **C17**, compiled with `gcc -std=c17 -Wall -Wextra -pedantic -g` plus **AddressSanitizer + UBSan**, and a **leak check** after every test.
  - Out-of-bounds access, use-after-free, NULL dereferences, signed overflow and **memory leaks all fail the test**, with a friendly explanation.
  - **Tests must free whatever the learner's functions allocate.**
- **No `main()`** in starters or solutions. The harness provides it. The learner's code and the tests are compiled as **one translation unit** (the learner's code comes first), so tests call the learner's functions directly.
- **`#include` what you use in both files.** The tests don't inherit the learner's headers. For example, `tests.c` needs `#include <stdlib.h>` if it calls `free`.
- **Reference solutions compile with zero warnings.** Starters may have warnings but no errors. To silence unused parameters in a starter, use `(void)param;`.
- Tests:
  ```c
  TEST(adds_two_numbers) {                // name shown as "adds two numbers"
      EXPECT_EQ(add(2, 3), 5);
  }
  ```
  Matchers stop the test at the first failure:
  - `EXPECT_EQ(a, e)` / `EXPECT_NE`: integers
  - `EXPECT_UEQ`: unsigned, shown in hex too
  - `EXPECT_TRUE` / `EXPECT_FALSE`
  - `EXPECT_NEAR(a, e, eps)`
  - `EXPECT_STR_EQ`: NULL-safe
  - `EXPECT_NULL` / `EXPECT_NOT_NULL` / `EXPECT_PTR_EQ`
  - `EXPECT_INT_ARRAY_EQ(a, e, n)`

  Because they `return` on failure, matchers only work directly inside a `TEST` body, not in helper functions that return a value.
- Each test runs in its own process with a **1.5 s** limit, so a crash only fails that one test.
- Prefer `size_t` for lengths, `const` for inputs you don't modify, and `bool` from `<stdbool.h>`.

## Bug fixes (`"kind": "fix"`, Fix-it Friday)

A fix challenge hands the learner **working-looking code with one bug** and a bug report. They read the code, find the bug and fix it. Every Friday the daily is a fix challenge; on other days fix challenges are never picked as the daily (they stay in the library and in spaced review).

- **Slug:** `l<level>-fix-<name>`, e.g. `c/l3-fix-word-count`. Set `"kind": "fix"` in `meta.json`. Everything else (level, rating band, topics, hints, estMinutes) works as for normal challenges.
- **`starter`** is the buggy code: complete, idiomatic-looking, compiles **warning-free** and passes **some** of the tests (the validator checks this). Exactly **one** bug, and it must be one a real person writes: off-by-one, `<` vs `<=`, integer division, a missing `break`, mutating the caller's data, a forgotten `await`, `sizeof` on a pointer, a shadowed variable, a missing null terminator… Don't leave `TODO`s or comments that point at the bug.
- **`solution`** is the starter with the bug fixed and **nothing else changed**. The validator reports how many lines the fix touches; keep it at 1–3.
- **`prompt.md`** is a bug report: a short story (who noticed what), **what the code is supposed to do** (the spec, with 2–3 input → output examples), and one concrete reproduction: "`f(x)` returns `y`, expected `z`". It describes the symptom, never the cause or the line.
- **`learn.md`** teaches the concept area the bug lives in (how loop bounds work, how C strings end, how `await` works) and one debugging technique (trace by hand with a small input, print intermediate values, check the boundaries). It must **not** say which line is wrong.
- **`topics`** name the concept area, never the bug ("loops", not "off-by-one"): they're shown on the task page. Reuse topic names from the language's existing challenges where they fit.
- **Hints:** 1) where to look or what to observe ("try it on a list with one item"), 2) which part of the code is wrong and why, 3) nearly the fix.
- **Tests** are hidden until the first run, so the bug report has to be enough to start.
- Level difficulty comes from the code and the bug: a level 1 fix is a 10-line function with an obvious symptom; a level 6 fix is a realistic 40–80 line piece of code where the bug is subtle (an edge case, aliasing, a race-free-looking async ordering, undefined behaviour).

## Reference examples

- TypeScript: `typescript/l1-hello-greeting`, `typescript/l6-my-pick`
- Python: `python/l1-hello-greeting`, `python/l4-bank-account`
- C: `c/l1-absolute-value`, `c/l4-min-max-out-params`
