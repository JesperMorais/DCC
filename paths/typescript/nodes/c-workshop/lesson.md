The expense tracker starts with an empty `main.ts` and a page of expected output. That blank file is the hardest part of any project, and it isn't a TypeScript problem: nobody writes a program from the top down in one go. This workshop is the craft between "I can write a function" and "I can build a program": how to start, how to check each piece, and what to do when the output is wrong. Take it slowly. Every step here is small on purpose.

The running example is a tiny CLI, `steps best steps.csv`, that prints your best walking day.

### Sketch the pipeline before you code

Write the program as a pipeline on paper first. Each arrow is a function, and only the two ends touch the outside world:

```
argv ──► read file ──► parse lines ──► validate ──► compute ──► format ──► print
 (I/O)                 Day | BadLine               best Day    string[]    (I/O)
```

Now you have five small problems instead of one big one, and you know what type flows between them. The middle is **pure**: data in, data out, no `console`, no files. Pure functions are the ones you can test on their own.

### Signature and one test, before the body

Pick the first box and write only its signature, with a body that throws:

```ts
// src/parse.ts
export interface Day { line: number; date: string; steps: number }
export interface BadLine { line: number; reason: string }

export function parseLine(text: string, line: number): Day | BadLine {
  throw new Error("not implemented");
}
```

Then one test that says what "right" looks like. **Arrange** the input, **act** by calling the function, **assert** on the result:

```ts
// tests/parse.test.ts
import { test } from "node:test";
import assert from "node:assert/strict";
import { parseLine } from "../src/parse.ts";

test("a good line becomes a Day", () => {
  const text = "2026-09-14,8412";                  // arrange
  const day = parseLine(text, 3);                   // act
  assert.deepEqual(day, { line: 3, date: "2026-09-14", steps: 8412 }); // assert
});
```

Run just that file with `node --import tsx --test tests/parse.test.ts`. It fails with `not implemented`, which is correct: the test works and the body doesn't exist yet. Here's a first attempt that forgot to convert the number:

```
✖ a good line becomes a Day
  AssertionError [ERR_ASSERTION]: Expected values to be strictly deep-equal:
  + actual - expected

    {
      date: '2026-09-14',
      line: 3,
  +   steps: '8412'
  -   steps: 8412
    }
```

Read it as "**+** is what you got, **-** is what the test wanted". The quotes give it away: a string, not a number. `deepEqual` compares objects and arrays field by field, which `===` can't. Fix, rerun, green. Then add the next test (a bad line), and only then the next branch of the body.

### Modules: one file per box

`export` makes a name visible to other files; `import` pulls it in. With this course's setup, imports name the `.ts` file:

```ts
import { parseLine, type Day } from "./parse.ts";
```

A good split follows the pipeline: `parse.ts`, `format.ts`, `commands.ts`, and a `main.ts` that is the only file doing I/O.

### Throw, or return a union?

Two kinds of trouble need two tools:

- **A bad line in the data** is expected. Return it as a value, `Day | BadLine`, and let the caller narrow with `"reason" in row`, exactly like Unions & narrowing. The caller can collect every bad line and keep going.
- **Something that ends the program** (the file doesn't exist) is a `throw`, caught once in `main`.

Without a catch, a missing file is a crash with a stack trace:

```
Error: ENOENT: no such file or directory, open 'nope.csv'
    at readFileSync (node:fs:440:20)
```

`try { … } catch (e) { … }` runs the `catch` block when anything inside `try` throws. The caught `e` has type `unknown` (anything can be thrown), so narrow it before use:

```ts
try {
  run(process.argv.slice(2));
} catch (e) {
  console.error(e instanceof Error ? `error: ${e.message}` : e);
  process.exitCode = 1;
}
```

This catches *every* error, including your own bugs, and prints them as if they were planned. In a bigger program, throw your own `class CliError extends Error {}` and check `e instanceof CliError`, so real bugs still crash loudly. (Classes get their own node in Generics; this one line is all you need for now.)

### stdout, stderr and the exit code

A program has two output streams. `console.log` writes to **stdout** (the results), `console.error` to **stderr** (warnings and errors). Scripts and tests read them separately, and the shell variable `$?` holds the **exit code**: 0 means success, anything else means failure.

```
$ npx tsx src/main.ts nope.csv
error: cannot read nope.csv
$ echo $?
1
$ npx tsx src/main.ts nope.csv 2>/dev/null     # hide stderr: nothing left
```

Set `process.exitCode = 1` and let the program finish, rather than calling `process.exit(1)` mid-way.

### Debugging when stdout is under test

The project's tests run your CLI and compare stdout line by line. So the classic `console.log("best is", best)` breaks the very test you're debugging:

```
    [
  +   "best is { date: '2026-09-14', steps: 8412 }",
      '2026-09-14  8412'
    ]
```

Use `console.error` for debug prints instead. You still see them in your terminal, and stdout stays exactly as the test expects. A few tests also check that stderr is empty on a clean run, so those stay red while your debug lines are in: delete them once you've found the bug.

### The small toolbox

The project needs a few methods the lessons haven't shown yet. Real outputs:

```ts
const DATE = /^(\d{4})-(\d{2})-(\d{2})$/;
DATE.test("2026-09-14");        // true      (does it match? boolean)
DATE.test("14/09/2026");        // false
DATE.exec("2026-09-14");        // ['2026-09-14', '2026', '09', '14', …] (the captures)
DATE.exec("nope");              // null

days.find((d) => d.steps > 8000);   // { date: '2026-09-14', steps: 8412 }  (first match)
days.find((d) => d.steps > 20000);  // undefined                           (no match)
days.some((d) => d.steps > 8000);   // true                                (any match?)

days.find((d) => d.steps > 20000)?.date;            // undefined, no crash
days.find((d) => d.steps > 20000)?.date ?? "none";  // "none"
```

`?.` stops and gives `undefined` if the left side is `undefined` or `null`, and `??` supplies a default. `find` returns `T | undefined`, so the compiler asks you to handle the miss.

### Gotchas

- **`filter` doesn't narrow on its own.** `rows.filter((r) => "reason" in r)` is still `(Day | BadLine)[]`. A `for...of` loop that pushes into two arrays narrows fine.
- **A regex with the `g` flag remembers where it stopped.** `/a/g.test("a")` gives `true`, then `false`, then `true`. Leave `g` off for "does this match?".
- **One test failing at a time is normal.** Read the first failure only, fix it, rerun.
- **Commit when green.** Each passing step is a version you can return to.

### In the wild

- **Git, npm and every Unix tool** print results to stdout, complaints to stderr, and return non-zero on failure. That's why `npm test && git push` stops when tests fail.
- **CI pipelines** (GitHub Actions, GitLab CI) decide pass or fail from the exit code alone.
- **Test-first work** at most companies looks like this: a signature, one failing test, the smallest body that passes, repeat. The tests become documentation.
- **Large codebases** keep I/O at the edges and pure logic in the middle, for exactly the reason you just saw: the middle is easy to test.
