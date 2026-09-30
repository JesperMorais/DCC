### Types can be exact values

`string` allows *any* text. Sometimes you want only a few specific values. You can write a
string itself as a type — a **literal type** — and combine several with `|` ("or"):

```ts
type Light = "red" | "yellow" | "green";

let now: Light = "green";  // OK
now = "blue";              // compile error: "blue" is not a Light
```

`type Name = ...` creates a **type alias** — a nickname you can reuse everywhere.

This union is called a **union of literals**. It turns typos into compile errors, and your
editor will autocomplete the allowed values.

### Comparing and combining

Inside the function, a `Light` is still a string, so you compare with `===`.
To express "this situation OR that situation", group each situation in parentheses:

```ts
if ((a === "red" && b === "green") || (a === "green" && b === "red")) { ... }
```

### Testing that something *fails* to compile

```ts
// @ts-expect-error
const oops: Light = "blue";
```

This line passes only if there **is** an error on the next line.
