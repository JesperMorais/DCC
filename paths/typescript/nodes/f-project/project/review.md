## How we'd structure it

### Three layers, one direction

```
main.ts ──► cli.ts ──► todo.ts      (pure: list in, new list out)
                  └──► storage.ts   (the only file that touches the disk)
```

- **`todo.ts`** knows nothing about files or the terminal. Every function takes a list and returns a value or a new list. That's why its tests are one-liners: no temp files, no fake input.
- **`storage.ts`** is the only place that reads or writes the disk. Both functions take the path as a parameter instead of hard-coding `"todos.json"`, so the tests can use a temp folder and never touch your real list.
- **`cli.ts`** is the glue: load, decide, call a pure function, save, return a message. `run` returns the message instead of printing it, so it's testable too, and `main.ts` is left with a single line that's too simple to break.

If you ever want a web page or a Discord bot for your to-dos, `todo.ts` and `storage.ts` move over untouched. Only the glue changes.

### The decisions that matter

**1. Return new lists, never change the old one.** `markDone` could flip `todos[i].done = true` and be "done". But then the caller's list changes under its feet, and a test (or a teammate) holding the old list sees the change too. The tests freeze their input to catch exactly this. Our version:

```ts
return todos.map((t) => (t.id === id ? { ...t, done: true } : t));
```

`{ ...t, done: true }` is object spread, the shortcut Core TypeScript shows you. Writing out `{ id: t.id, text: t.text, done: true }` is just as correct. Marking the parameters `readonly Todo[]` makes the compiler refuse a `push` or `splice`, so a mistake shows up before you even run the tests.

**2. Ids are stable, not positions.** We use "highest id + 1", not "length + 1" and not the position in the list. If you remove to-do 2 from `[1, 2, 3]`, the next one is 4, and "3" still means the same to-do it meant a minute ago. With `length + 1` you'd get a second to-do 3. With positions, `done 3` after a removal would hit the wrong item.

**3. Validate at the edge, trust inside.** The pure functions assume a sensible number. `run` is where text from the user comes in, so that's where we check for a missing id (`"Which to-do?"`) and an id that doesn't exist. A nice detail: `Number("abc")` is `NaN`, and `NaN` equals nothing, not even itself, so "abc" falls into the same "No to-do with id" branch as 7 with no extra code.

We look the to-do up *before* changing anything, because the message needs its text (`Done: buy milk`) and because "nothing happened" should never be saved.

### Smaller things worth copying

- `addTodo` returns the list unchanged for blank text, and `run` spots that by comparing lengths. One rule, written once.
- `summary` checks "empty" first. Checked last, an empty list would hit "all done" first (0 of 0 left is technically all done) and print `All 0 done!`.
- The JSON is saved with 2-space indentation. Nobody needs it to be pretty, but when something goes wrong you'll be glad you can open `todos.json` and read it.

### Where it goes from here

In Core TypeScript you'll describe this data with an `interface`, make a command a union like `{ kind: "add"; text: string } | { kind: "done"; id: number }`, and let the compiler check that `run` handles every kind.
