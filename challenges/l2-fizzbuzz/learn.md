### Building up an array

Start with an empty array and add to it with `push`:

```ts
const squares: number[] = [];
for (let i = 1; i <= 3; i++) {
  squares.push(i * i);
}
// squares is [1, 4, 9]
```

Note the type annotation `number[]` on the empty array. Without it, TypeScript can't
guess what will go into `[]` later.

### "Divisible by"

`a % b === 0` means "`b` goes into `a` with nothing left over":

```ts
12 % 4 === 0; // true  — 12 is divisible by 4
10 % 4 === 0; // false — remainder is 2
```

### Number → string

A `string[]` can only hold strings, so numbers must be converted:

```ts
String(42);  // "42"
`${42}`;     // "42"
```

### Order of checks

In an `if / else if` chain, only the **first** matching branch runs. When one condition is a
special case of another, think carefully about which to test first.
