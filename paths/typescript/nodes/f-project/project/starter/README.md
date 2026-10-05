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
npm run test:m1      # test one milestone (m1 … m5)
npm test             # every test
npm run check        # type-check everything
npm start list       # run the app (after milestone 5)
```

Work through the milestones in order. Each one builds on the one before, and `npm run test:mN` tells you when you're done.

The app keeps its list in `todos.json` in the folder you run it from. Delete the file to start over.

## Toolbox: things Fundamentals didn't cover

You'll need a few built-ins that the lessons haven't shown. This is what they do. Working out where they go is up to you.

| Built-in | What it does |
|---|---|
| `lines.join("\n")` | glues an array of strings into one string, with `"\n"` between them |
| `Number("2")` | turns text into a number: `2`. `Number("abc")` is `NaN`, which never equals anything |
| `JSON.stringify(value, null, 2)` | turns a value into JSON text, indented 2 spaces so it's readable |
| `JSON.parse(text)` | turns JSON text back into a value |
| `fs.existsSync(file)` | `true` if the file is there |
| `fs.readFileSync(file, "utf8")` | the file's contents as a string |
| `fs.writeFileSync(file, text)` | writes the string to the file, replacing what was there |

`fs` is already imported in `src/storage.ts`.
