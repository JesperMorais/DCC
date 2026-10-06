# todo-cli

A to-do list that lives in your terminal and remembers between runs.

```sh
npm start add buy milk     # Added 1. buy milk
npm start list             # [ ] 1. buy milk …
npm start done 1           # Done: buy milk
```

## What's here

| File | What you do there |
|---|---|
| `src/todo.ts` | The `Todo` type and the pure list functions (milestones 1–3) |
| `src/storage.ts` | Load and save the list as JSON (milestone 4) |
| `src/cli.ts` | `run`: turn the typed words into an action and a message (milestone 5) |
| `src/main.ts` | Already done. Calls `run` and prints the result |
| `tests/m1.test.ts` … `tests/m5.test.ts` | One test file per milestone. Read them: they're the spec |

Every function you need to write is already there with its signature and a comment saying what it should do. It throws `TODO` until you replace the body.

## Commands

```sh
npm install          # once
npm run test:m1      # type-check, then test one milestone (m1 … m5)
npm run test:one -- "gaps" tests/m1.test.ts   # type-check, then just the tests whose name contains "gaps"
npm test             # type-check, then every test
npm run check        # type-check only
npm start list       # run the app (after milestone 5)
```

Every test script runs `tsc --noEmit` first. If there's a type error anywhere in `src/` or `tests/`, you'll see the error (`src/todo.ts:18:23 - error TS2322: …`) and **no tests run** until it's fixed. That's on purpose: a wrong type caught here is much easier to fix than the strange failure it would cause later.

Work through the milestones in order. Each one builds on the one before, and `npm run test:mN` tells you when you're done.

The app keeps its list in `todos.json` in the folder you run it from. Delete the file to start over.

## Toolbox: things Fundamentals didn't cover

You'll need a few built-ins that the lessons haven't shown. This is what they do. Working out where they go is up to you.

| Built-in | What it does |
|---|---|
| `export function f…` / `import { f } from "./todo.ts"` | shares code between files. Only what a file `export`s can be imported; the path is relative to the importing file and ends in `.ts`. `import type { Todo }` imports just a type |
| `lines.join("\n")` | glues an array of strings into one string, with `"\n"` between them |
| `Number("2")` | turns text into a number: `2`. `Number("abc")` is `NaN`, which never equals anything |
| `JSON.stringify(value, null, 2)` | turns a value into JSON text, indented 2 spaces so it's readable |
| `JSON.parse(text)` | turns JSON text back into a value |
| `fs.existsSync(file)` | `true` if the file is there |
| `fs.readFileSync(file, "utf8")` | the file's contents as a string |
| `fs.writeFileSync(file, text)` | writes the string to the file, replacing what was there |

`fs` is already imported in `src/storage.ts`.

## When you're stuck

1. **Read the first failing test only.** Scroll to `✖ failing tests:` and take the first one. Under `+ actual - expected`, the `+` lines are what your function returned and the `-` lines are what the test wanted; unmarked lines matched. The line `at … tests/m1.test.ts:19:10` is the assertion that failed: open it and read the input. Skip the `node:internal` lines, and don't trust the line number in the `test at tests/m1.test.ts:1:476` header.
2. **Run just that milestone** (`npm run test:m2`), or one test by name: `npm run test:one -- "gaps" tests/m1.test.ts` (part of the test's name, then its file).
3. **Look at the value.** Add `console.error("addTodo:", todos, clean)` and run the test again: it prints just above the result. Delete it once you've seen what you needed.
4. **Make the step smaller.** One function, then one test, then one line. Change one thing, run, look.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each milestone, so you can always get back to a working version. `.gitignore` already keeps `node_modules/` and your `todos.json` out.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.
