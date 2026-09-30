# Writing challenges

Each challenge is a folder: `challenges/l<level>-<slug>/`. The folder name is the id.

| File | What it holds |
|---|---|
| `meta.json` | `title`, `level` (1–6), `rating`, `topics[]`, `estMinutes`, `mode` (`"runtime"` or `"types"`), `hints[]` |
| `prompt.md` | The task. Markdown. Short, concrete, with 2–3 input → output examples. |
| `learn.md` | A mini-lesson on the TypeScript concept the challenge practices (always visible, the "Concept" tab). 60–200 words plus a small code example. It teaches the idea; it does **not** give away the solution. |
| `starter.ts` | What the learner starts with. Must type-check on its own and must **fail** the tests. |
| `solution.ts` | Reference solution. Must pass. Shown after solving / giving up. |
| `tests.ts` | Tests, visible to the learner. |

Run `npm run validate` (or `npm run validate -- l3-` to filter) — it must report 0 problems.

## Rules

- **No `import` / `export`.** `starter.ts`/`solution.ts` and `tests.ts` are global scripts that share one scope. Tests just call the learner's functions/types by name.
- Compiled with `strict: true`, lib ES2023, no DOM. Globals available: `console`, `setTimeout`, `clearTimeout`, `queueMicrotask`, `structuredClone`.
- The test signature types matter: tests are type-checked together with the learner's code, so a wrong parameter/return type shows up as a compile error. That's intended — it teaches TypeScript.
- **Target time: ~10 minutes** for someone at that level (`estMinutes` 5–12).
- **Hints**: 2–3 of them, escalating. First = nudge, second = approach, last = almost the answer.
- Keep function/type names unique across the whole bank-ish (not required, but nice).

### Test API (`mode: "runtime"`)

```ts
test("name", () => { expect(actual).toBe(expected); });       // also: async () => { ... }
// matchers: toBe, toEqual (deep), toBeCloseTo(n, digits?), toBeTruthy, toBeFalsy,
// toBeUndefined, toBeNull, toBeGreaterThan, toBeLessThan, toHaveLength, toContain,
// toBeInstanceOf, toThrow(msg?)  — and .not.<matcher>
```

`expect<T>(actual: T)` is generic — `toBe`/`toEqual` want the same type as `actual`. Aim for 4–7 tests per challenge including edge cases (empty input, etc.).

### Type tests (`mode: "types"`)

Judged by the compiler only — pass = zero type errors. Helpers available globally:

```ts
type cases = [
  Expect<Equal<MyType<X>, Expected>>,
  ExpectFalse<Equal<A, B>>,
];
// @ts-expect-error  ← the next line MUST be a type error
type bad = MyType<Invalid>;
```

`Equal`, `NotEqual`, `Expect`, `ExpectFalse`, `IsAny` exist. The starter is usually `type Foo<T> = any;` (which fails `Equal`).
Runtime challenges may also include `Expect<...>` type assertions for signatures.

## Levels & rating bands

The app uses an Elo-style rating to pick the next challenge, so `rating` should honestly reflect difficulty inside the band.

| Level | Name | Rating band | Audience |
|---|---|---|---|
| 1 | First steps | 700–850 | Never coded. Variables, basic types, if/else, simple functions, string basics. |
| 2 | Building blocks | 850–1000 | Loops, arrays, objects, `for...of`, basic array methods, union of literals. |
| 3 | Shaping data | 1000–1150 | Interfaces/type aliases, optional props, unions + narrowing, `map/filter/reduce`, Records, tuples. |
| 4 | Generics | 1150–1300 | Generic functions, constraints, `keyof`, Map/Set, classes, discriminated unions, utility types (`Partial`, `Pick`, `Omit`, `Readonly`). |
| 5 | Advanced patterns | 1300–1450 | Async/Promises, closures & higher-order functions, type guards/`asserts`, overloads, Result types, algorithms (memoize, debounce-like, parsing, recursion). |
| 6 | Type wizardry | 1450–1650 | Conditional types, `infer`, mapped types with `as`, template literal types, recursive types — mostly `mode: "types"`. |

See `l1-hello-greeting` and `l6-my-pick` for reference examples.
