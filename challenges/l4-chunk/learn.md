### Generic functions

A type parameter is a placeholder for "whatever type the caller uses". TypeScript fills it in from the arguments:

```ts
function last<T>(items: readonly T[]): T | undefined {
  return items[items.length - 1];
}

last([1, 2, 3]);      // T = number  → number | undefined
last(["x", "y"]);     // T = string  → string | undefined
```

Compare with `any`:

```ts
function lastAny(items: any[]): any { /* ... */ }
const v = lastAny([1, 2]); // any — typos like v.toFixd() are no longer caught
```

`any` switches the type checker off; a generic keeps the connection between what goes **in** and what comes **out**.

### Building nested types

If one element is `T`, a list of them is `T[]`, and a list of lists is `T[][]`. `slice(start, end)` returns a new array and never goes out of bounds — a too-large `end` just stops at the end of the array.
