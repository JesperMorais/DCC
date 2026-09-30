### Arrays

An **array** is an ordered list of values. Its type is written `number[]` ("array of numbers"),
`string[]`, and so on:

```ts
const temps: number[] = [12, 18, 9];
temps[0];      // 12  — the first item is at index 0
temps.length;  // 3
```

### Looping over an array

`for...of` visits every element in order:

```ts
for (const t of temps) {
  console.log(t); // 12, 18, 9
}
```

### Tracking a "best so far"

A common trick when searching: remember the best candidate in a `let` variable
and update it whenever you find something better.

```ts
const words = ["hi", "hello", "hey"];
let longest = words[0];
for (const w of words) {
  if (w.length > longest.length) {
    longest = w;
  }
}
// longest is "hello"
```

Watch out for the starting value: starting with `0` only seems to work — think about what
happens if every number is negative.
