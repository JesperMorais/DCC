# typed-store

A typed store with tests you write yourself, an event bus under it, and a small shopping-cart
CLI on top. The milestones on the project page say what has to work. The file layout, the
types, the signatures and the tests are all yours to design.

Take it slowly. Nobody is timing this, and every hour you spend stuck on one milestone is an
hour of exactly the practice this project exists for.

## Commands

```sh
npm install        # once
npm start          # runs src/main.ts
npm test           # type-checks, then runs every tests/**/*.test.ts with Node's test runner
npm run test:m1    # type-checks, then runs only tests/m1-*.test.ts (test:m2 … test:m6 likewise)
npm run test:one -- "once" tests/m2-bus.test.ts   # type-checks, then runs one test by part of its name
npm run check      # type-checks src/ AND tests/ (this is what verifies your type-level tests)
```

**The test scripts type-check first.** `npm test` and `npm run test:mN` run `tsc --noEmit`
before any test, and stop if it fails. So when a run ends with `error TS…` lines and no test
results, nothing is wrong with your tests' logic yet: fix the type error it names (file, line,
column) and run again. That is also how a broken `@ts-expect-error` line shows up.

Name your test files after the milestone, `tests/m1-store.test.ts`, `tests/m2-bus.test.ts`
and so on, so `npm run test:mN` finds them.

## Writing tests

This project has no given tests: writing them is part of the job (the workshop before this
project shows how). `tests/example.test.ts` shows the syntax:

- `import { test } from "node:test";` and `import assert from "node:assert/strict";`
- `assert.equal` (`===`), `assert.deepEqual` (same structure), `assert.throws(fn, ErrorClass)`,
  `assert.ok`, `assert.match(text, /regex/)`
- **type-level tests**: a line marked `// @ts-expect-error` must fail to compile. If it
  compiles, `npm run check` fails with "Unused '@ts-expect-error' directive". It accepts *any*
  error, so write the line without the directive first, read tsc's message, check it's the
  error you meant, then add the directive. Keep one line that *should* compile next to them.
- **exact types**: `tests/type-helpers.ts` exports `Equal` and `Expect`.
  `type _cases = [Expect<Equal<typeof volume, number>>];` fails to compile unless the two
  types are identical.

Put type-level lines inside a function you never call, so they only matter to the type checker,
and end the file with `void typeChecks;`. `void` evaluates the name and discards it: the function
still never runs, but your editor stops flagging it as unused.

Each milestone lists what your tests should cover. A milestone is done when those tests exist
and `npm test` is green (which includes the type check).

## Toolbox

Things the milestones need that the lessons only touched on.

**`Object.keys` and type assertions.** `Object.keys(patch)` is typed `string[]`, because an
object may have more keys at runtime than its type lists. When you know better, say so with an
assertion: `Object.keys(patch) as (keyof S)[]`. An assertion changes only what TypeScript
believes, it checks nothing at runtime, so use it on a line you've thought about.

**Types from values: `keyof typeof`.** `typeof CATALOG` (in a type position) is the type of the
`CATALOG` object, and `keyof typeof CATALOG` is the union of its keys. Declare the object
`as const` and the keys stay literal: `"apple" | "bread" | …`. The value is then the single
source of truth.

**Reading lines from stdin** with `node:readline`:

```ts
import readline from "node:readline";

const rl = readline.createInterface({ input: process.stdin });
for await (const line of rl) {
  if (line === "quit") break; // leaving the loop closes rl
  console.log(`you said: ${line}`);
}
```

Try it with `printf 'hi\nquit\n' | npm start`. `process.stdin.isTTY` tells you whether a person
is typing (show a prompt) or input is piped in (don't).

**Running the CLI from a test** with `node:child_process`:

```ts
import { execFileSync } from "node:child_process";

const out = execFileSync(process.execPath, ["--import", "tsx", "src/main.ts"], {
  input: "add apple 3\nquit\n",
  encoding: "utf8",
});
```

`out` is everything the program printed to stdout, as one string. `assert.match` it.

## When you're stuck

1. **Read the first failing test only.** In the summary at the bottom, find the first `✖`. Under
   it, `+ actual - expected` shows the difference: lines with `+` are what your code produced,
   lines with `-` are what the test wanted. The `at TestContext.<anonymous> (…/m2-bus.test.ts:52:10)`
   line is the assert that failed. For example:

   ```
   ✖ once fires a single time
     AssertionError [ERR_ASSERTION]: Expected values to be strictly deep-equal:
     + actual - expected

       [
         'ada',
     +   'grace'
       ]
   ```

   Your listener ran for the second emit too, so whatever should remove it didn't.
   If the run stopped at `error TS…` instead, it's the type check: read that first.
2. **Run just that milestone** (`npm run test:m2`), or one test by name:
   `npm run test:one -- "once" tests/m2-bus.test.ts` (part of the test's name, then its file).
3. **Look at the value.** Add `console.error(value)` where you're unsure what's there. In the CLI,
   stdout is what your tests check and stderr isn't, so `console.error` never breaks an output
   assertion.
4. **Make the step smaller.** One function, one test, one line at a time. Write the test for the
   next tiny behaviour, see it fail, make it pass.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each
   milestone, so you can always get back to a working version.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.

## Layout

There is one placeholder, `src/main.ts`. Add your own files under `src/` and import them
with the `.ts` extension, for example `import { Store } from "../src/store.ts";` from a test.
Only Node's built-in modules are available (`node:readline`, `node:fs`, `process.argv`, …).
