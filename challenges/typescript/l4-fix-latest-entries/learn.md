### Generic constraints

`<T extends { at: number }>` means "any type, as long as it has a numeric `at`". Inside the function you can read `entry.at`, and the caller gets their own type back: pass in log lines, get log lines out, with every extra field intact.

```ts
function newest<T extends { at: number }>(items: readonly T[]): T | undefined {
  return [...items].sort((a, b) => b.at - a.at)[0];
}
```

### `slice` and negative indexes

`slice(start, end)` copies part of an array. A **negative** index counts from the end:

```ts
const xs = ["a", "b", "c", "d"];
xs.slice(1);    // ["b", "c", "d"]
xs.slice(-2);   // ["c", "d"]
xs.slice(-10);  // ["a", "b", "c", "d"]  (clamped to the start)
```

Handy, but the "from the end" trick only works for numbers that really are negative.

### Debugging technique: test the boundaries

Most bugs hide at the edges, not in the middle. When a function takes a count, run it with the largest sensible value, with `1`, and with `0`. Write down what you *expect* first, then compare. An edge where the expected answer is "nothing" is the one people forget.
