### Numbers

In TypeScript, any number — whole (`7`), negative (`-3`) or with decimals (`2.5`) — has the type **`number`**.

### Doing math

You write math almost like on a calculator:

| Symbol | Meaning  | Example   | Result |
|--------|----------|-----------|--------|
| `+`    | add      | `4 + 2`   | `6`    |
| `-`    | subtract | `4 - 2`   | `2`    |
| `*`    | multiply | `4 * 2`   | `8`    |
| `/`    | divide   | `4 / 2`   | `2`    |

Just like in school math, `*` and `/` happen **before** `+` and `-`.
Parentheses `( )` let you choose the order yourself:

```ts
2 + 3 * 4;   // 14
(2 + 3) * 4; // 20
```

### Using a parameter

Inside a function, the parameter name stands for whatever number was passed in:

```ts
function minutesToSeconds(minutes: number): number {
  return minutes * 60;
}

minutesToSeconds(2); // 120
```
