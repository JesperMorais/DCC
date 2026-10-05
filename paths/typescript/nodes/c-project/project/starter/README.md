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
npm run test:m1     # one milestone
npm test            # everything
npm run check       # type-check with tsc
```

The tests run `src/main.ts` as a separate process with the files in `tests/fixtures/` and compare what it prints (stdout and stderr) and its exit code. They don't import your code, so you can structure it however you like. Read a test file when you're unsure what's expected: the exact lines are in there.

## Files

- `src/types.ts`: the data types (given). Use them; add your own as you need.
- `src/main.ts`: the entry point. It's a stub for now.
- `tests/`: one test file per milestone, `helpers.ts` that runs the CLI, and `fixtures/` with sample exports and rules files.

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
