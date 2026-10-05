# typed-store

A typed event bus, a store built on top of it, and a small shopping-cart CLI that uses both.
The milestones on the project page say what has to work. The file layout, the types, the
signatures and the tests are all yours to design.

## Commands

```sh
npm install        # once
npm start          # runs src/main.ts
npm test           # runs every tests/**/*.test.ts with Node's built-in test runner
npm run check      # type-checks src/ AND tests/ (this is what verifies your type-level tests)
```

Run a single test file with `node --import tsx --test tests/bus.test.ts`.

## Writing tests

This project has no given tests: writing them is part of the job. `tests/example.test.ts`
shows everything you need:

- `import { test } from "node:test";` and `import assert from "node:assert/strict";`
- `assert.equal`, `assert.deepEqual`, `assert.throws`, `assert.ok`
- **type-level tests**: a line marked `// @ts-expect-error` must fail to compile. If it
  compiles, `npm run check` fails with "Unused '@ts-expect-error' directive". Put these
  lines inside a function you never call, so they only matter to the type checker.

Each milestone lists what your tests should cover. A milestone is done when those tests
exist, `npm test` is green **and** `npm run check` is clean.

## Layout

There is one placeholder, `src/main.ts`. Add your own files under `src/` and import them
with the `.ts` extension, for example `import { Store } from "../src/store.ts";` from a test.
Only Node's built-in modules are available (`node:readline`, `node:fs`, `process.argv`, …).
