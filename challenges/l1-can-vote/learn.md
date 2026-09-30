### Combining true/false values

Often a decision depends on more than one thing. TypeScript has **logical operators**
to combine booleans:

- `&&` means **and** — `true` only when **both** sides are true
- `||` means **or** — `true` when **at least one** side is true
- `!` means **not** — it flips a boolean: `!true` is `false`

```ts
const sunny = true;
const weekend = false;

sunny && weekend; // false — not both
sunny || weekend; // true  — at least one
!weekend;         // true
```

### More comparisons

```ts
7 > 5;   // true   greater than
5 >= 5;  // true   greater than OR equal to
3 < 5;   // true   less than
5 <= 4;  // false  less than OR equal to
```

A boolean parameter (like `isCitizen: boolean`) is already `true` or `false`,
so you can use it directly — no need to write `=== true`.
