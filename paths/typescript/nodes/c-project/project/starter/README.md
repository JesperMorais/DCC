# Expense tracker CLI

Reads a bank's CSV export, sorts transactions into categories and reports where the money went. The milestones (in the app) describe every command and its exact output.

## Run it

```sh
npm install
npm start -- list tests/fixtures/clean.csv
npm start -- report tests/fixtures/clean.csv 2026-09 --rules tests/fixtures/rules.txt
```

Everything after `--` is passed to your program as `process.argv.slice(2)`.

## Test it

```sh
npm run test:mine   # your own unit tests (tests/mine.test.ts)
npm run test:one -- "month" tests/m3.test.ts   # type-check, then only tests whose name contains "month"
npm run test:m1     # one milestone
npm test            # everything
npm run check       # type-check with tsc
```

The milestone tests run `src/main.ts` as a separate process with the files in `tests/fixtures/` and compare what it prints (stdout and stderr) and its exit code. They don't import your code, so you can structure it however you like. Read a test file when you're unsure what's expected: the exact lines are in there.

`tests/mine.test.ts` is different: it imports one function and checks it directly. It starts with one example test for the `parseRow` suggested in `src/csv.ts`; the rest are yours to write.

## Where to start

Don't start in `main.ts`. Start with one line of the CSV: `parseRow` in `src/csv.ts` and its test in `tests/mine.test.ts`. When `npm run test:mine` is green, `main.ts` only has to read the file, loop over the lines and print. Sketch the rest the same way before you write it: read → parse → check → compute → format → print, one function per arrow.

## Toolbox

A few things this project needs that the Core lessons only touch on. The **Workshop: from functions to a program** node, just before this project, shows each one with real output:

- `try { … } catch (e) { … }` to turn a failed file read into an error message. `e` is `unknown`, so narrow it (`e instanceof Error`).
- `throw` for problems that end the program; returning a union (`Transaction | BadRow`) for problems you collect and keep going after.
- `process.exitCode = 1`, `console.log` (stdout, the results) and `console.error` (stderr, warnings and errors).
- `regex.test(text)` (does it match?) and `regex.exec(text)` (the captured parts, or `null`).
- `array.find(…)` (the first match, or `undefined`), `array.some(…)` (is there any match?), and `?.` / `??` for "maybe missing".
- `node:test` and `assert.deepEqual` for your own unit tests.

## When you're stuck

1. **Read the first failing test only.** Scroll up to the first `✖`. Under `+ actual - expected`, the `+` lines are what your program printed and the `-` lines are what the test wanted; lines without a sign matched. An `assert.equal(r.code, 0)` failure means your program exited with an error: look at stderr in the same output.
2. **Run just that milestone** (`npm run test:m2`), or one test by part of its name: `npm run test:one -- "trimmed" tests/m2.test.ts`. Or run the command yourself: `npm start -- list tests/fixtures/messy.csv`.
3. **Look at the value.** Print it with `console.error("rows", rows)`, never `console.log`: stdout is what the tests compare, stderr is just shown to you. (A few tests check that stderr is empty on a clean run, so delete your debug lines when you're done.) Better still, write a test in `tests/mine.test.ts` that calls the function with that exact input.
4. **Make the step smaller.** One function, one test, one line at a time. If a whole command is wrong, check each step of its pipeline on its own.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each milestone, so you can always get back to a working version.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.

## Files

- `src/types.ts`: the data types (given). Use them; add your own as you need.
- `src/csv.ts`: a suggested first function, `parseRow`. A stub for now.
- `src/main.ts`: the entry point. It's a stub for now.
- `tests/`: one test file per milestone, `mine.test.ts` for your own tests, `helpers.ts` that runs the CLI, and `fixtures/` with sample exports and rules files.

A layout that works well (a suggestion, not a requirement):

```
src/
  types.ts     the data types
  csv.ts       reading the export
  rules.ts     reading the rules file and categorising
  format.ts    turning numbers into text
  commands.ts  one function per command
  main.ts      arguments, files, printing and exit codes
```
