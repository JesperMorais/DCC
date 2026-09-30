### Repeating work with a `for` loop

A `for` loop runs the same block of code many times. It has three parts, separated by `;`:

```ts
for (let i = 0; i < 3; i++) {
  console.log(i); // prints 0, then 1, then 2
}
```

1. **start** — `let i = 0` creates a counter
2. **keep going while** — `i < 3` is checked before every round
3. **step** — `i++` (add 1 to `i`) runs after every round

Change `<` to `<=` to include the last number, or start at a different value.

### The accumulator pattern

To combine many values into one, keep a `let` variable outside the loop and update it inside:

```ts
let product = 1;
for (let i = 1; i <= 4; i++) {
  product = product * i;  // or the shortcut: product *= i
}
// product is now 24
```

`x += 5` is shorthand for `x = x + 5`.
