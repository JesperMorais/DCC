Until now, every line you wrote ran in a box on a web page. Real projects don't live there. They live in a folder on your computer, you edit them in your own editor, and you run them from a terminal. That switch feels big the first time, and it's mostly a handful of commands. This workshop walks you through them once, with the next project's starter, so the project itself can be about the code.

Take it slowly. Type each command yourself and look at what comes back. Nothing here is timed.

### 1. Install Node and meet the terminal

Install **Node.js 22 or newer** from nodejs.org (the LTS button). On Windows, work inside **WSL** (Ubuntu from the Microsoft Store) or **Git Bash**, so the commands below work exactly as written. Then open a terminal and check:

```
$ node --version
v22.22.1
```

Any `v22` or higher is fine. "command not found" means the install didn't finish or the terminal was open before it: close it and open a new one.

Five things cover most of what you'll do in a terminal:

| Type | It does |
|---|---|
| `pwd` | prints the folder you're in ("print working directory") |
| `ls` | lists what's in it |
| `cd todo-cli` / `cd ..` / `cd ~` | moves into a folder / up one / home |
| `mkdir code` | makes a folder |
| **Tab** / **Ctrl+C** | completes a half-typed name / stops whatever is running |

Press Tab constantly: it saves typing and catches typos, because it only completes names that exist.

### 2. Get the starter and open it

The project page shows one command. It looks like this:

```
mkdir -p ~/code && cp -r ".../starter" ~/code/todo-cli && cd ~/code/todo-cli && npm install
```

Read it as four steps joined by `&&` ("and if that worked, then"): make `~/code`, copy the starter into it, move into the copy, install the tools it needs. You'll see:

```
added 6 packages, and audited 7 packages in 2s
found 0 vulnerabilities
```

Now open the folder in an editor. With VS Code installed, `code .` opens the folder you're in (`.` means "here").

### 3. What's in the folder

```
todo-cli/
  package.json     name, tools, and the "scripts" you run
  src/             your code
  tests/           the tests: the spec for each milestone
  node_modules/    the installed tools. Never edit, never commit
  README.md        how to run things. Read it first
```

`package.json` has a `"scripts"` section. `npm run test:m1` runs the script called `test:m1`; `npm start` and `npm test` are shortcuts for `start` and `test`.

The code is split across files, and files share code with `export` and `import`:

```ts
// src/todo.ts
export type Todo = { id: number; text: string; done: boolean };
export function addTodo(todos: readonly Todo[], text: string): Todo[] { … }

// src/cli.ts
import type { Todo } from "./todo.ts";
import { loadTodos, saveTodos } from "./storage.ts";
```

Only what a file `export`s can be imported elsewhere. The path is relative to the importing file, and in these projects it ends in `.ts`.

### 4. Run the tests and read the red

```
$ npm run test:m1
```

On a fresh starter every test fails with `Error: TODO (m1): addTodo`. That's expected: the functions are stubs. Here's a more interesting failure, after a first attempt at `addTodo`:

```
✖ addTodo gives the next id after the highest one, even with gaps (7.8ms)
ℹ tests 7
ℹ pass 4
ℹ fail 3

✖ failing tests:

test at tests/m1.test.ts:1:476
✖ addTodo gives the next id after the highest one, even with gaps (7.8ms)
  AssertionError [ERR_ASSERTION]: Expected values to be strictly deep-equal:
  + actual - expected

    {
      done: false,
  +   id: 3,
  -   id: 5,
      text: 'c'
    }

      at TestContext.<anonymous> (/home/you/code/todo-cli/tests/m1.test.ts:19:10)
      at Test.runInAsyncScope (node:async_hooks:214:14)
      at Test.run (node:internal/test_runner/test:1047:25)
```

How to read it:

- **Start with the first failure only.** Fixing one often fixes the next three.
- **The test name** says what was being checked.
- **`+ actual`** is what your code returned, **`- expected`** is what the test wanted. Here: your id was `3`, it should be `5`. Lines without a sign match.
- **The `at` frame in your own file** is the one that matters: `tests/m1.test.ts:19:10`. Open the test at line 19 and read its input. Skip every `node:internal` and `node:async_hooks` line: that's Node's machinery, not your code.
- **`test at tests/m1.test.ts:1:476`** in the header can point at the wrong line (here, line 1). Trust the `at` frame instead.

To run just that one test, give `test:one` a piece of its name and the file. It type-checks first, like the others:

```
$ npm run test:one -- "gaps" tests/m1.test.ts
```

### 5. Type errors come first

The test scripts run `tsc --noEmit` **before** the tests. If your code has a type error, you get this and no tests at all:

```
src/todo.ts:18:23 - error TS2322: Type 'string' is not assignable to type 'number'.

18   return [...todos, { id: String(todos.length + 1), text: clean, done: false }];
                         ~~
```

That's on purpose. `tsx` runs TypeScript **without** checking types, so a wrong type could slip through and fail somewhere confusing, or pass by luck. Reading a type error is quicker than chasing its effects. File, line and column are at the start; the `~~~` marks the spot. Your editor shows the same errors as red squiggles while you type.

### 6. Look at the values

When you can't see why a value is wrong, print it. `console.error` writes to the terminal and shows up right above the test result:

```
addTodo: [ { id: 1, text: 'a', done: false }, { id: 4, text: 'b', done: true } ] c
✖ addTodo gives the next id after the highest one, even with gaps
```

Prefer `console.error` over `console.log` for this kind of peeking: later projects check what your program prints to stdout, and stderr is left alone. Remove the line once you've seen what you needed. Once the app is wired up, `npm start add buy milk` runs it for real.

### 7. Save your progress with git

```
$ git init
$ git add -A
$ git status --short
A  .gitignore
A  package.json
A  src/todo.ts
…
$ git commit -m "m1 green"
```

The starter's `.gitignore` already lists `node_modules/`, so the installed tools stay out. Commit every time a milestone goes green. Then if an experiment makes everything worse, `git restore .` takes you back to the last green.

### 8. The smallest next step

When a milestone feels too big, shrink it: one function, then one test, then one line. Make a change, run the tests, look. Getting stuck and unstuck is the actual skill a project trains, and the hints are part of the kit, not a penalty. If nothing moves for a while, walk away for ten minutes. A lot of bugs get solved on the way back to the desk.

### In the wild

- Every professional TypeScript project has a `package.json` with scripts like these. "Run `npm test`" is the first instruction in most READMEs.
- CI systems (GitHub Actions, GitLab CI) do exactly what your test script does: type-check, then test, and refuse to merge on red.
- "Small commits on green" is how teams keep their main branch working: when something breaks, they can find the commit that broke it.
