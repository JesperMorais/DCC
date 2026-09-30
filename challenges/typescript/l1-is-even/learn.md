### Booleans: true or false

Some questions only have two answers: yes or no. In code those answers are the values
**`true`** and **`false`**, and their type is **`boolean`**.

### Comparisons give you booleans

When you compare two values, the result is a boolean:

```ts
5 > 3;     // true
5 === 3;   // false  (=== means "is exactly equal to")
5 !== 3;   // true   (!== means "is not equal to")
```

Because a comparison *is* a boolean, a function can simply return it:

```ts
function isPositive(n: number): boolean {
  return n > 0;
}
```

### The remainder operator `%`

`a % b` gives the **remainder** — what is left over when you divide `a` by `b`:

```ts
10 % 3; // 1   (3 fits 3 times into 10, 1 left over)
9 % 3;  // 0   (3 fits exactly, nothing left)
```
