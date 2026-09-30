### What is a function?

A **function** is a small machine: you give it an input, it gives you back an output.

```ts
function addExclamation(word: string): string {
  return word + "!";
}

addExclamation("wow"); // "wow!"
```

- `word` is the **parameter** — the name for the input.
- `: string` is the **type**. It tells TypeScript "this is text".
- `return` hands the answer back.

### Strings and their methods

Text in code is called a **string** and is written in quotes: `"hello"`.
Strings have built-in helpers called **methods**. You use them with a dot:

```ts
const name = "Ada";
name.length;        // 3 (how many characters)
name.includes("d"); // true
```

You can also glue strings together with `+`: `"ab" + "cd"` is `"abcd"`.

Look through the helpers your editor suggests when you type `text.` — there are ones for changing letter size!
