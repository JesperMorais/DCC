### Tuples are just special arrays

A tuple type fixes both the **length** and the **type at each position**:

```ts
type Pair = [string, number];
type P0 = Pair[0];         // string
type Len = Pair["length"]; // 2   — a literal, not just `number`!
type Any = Pair[number];   // string | number — indexing with `number` = "any position"
```

For plain arrays like `string[]`, `length` is just `number` — the length isn't known.

### `as const` makes readonly tuples

```ts
const dirs = ["up", "down"] as const;
type Dirs = typeof dirs;   // readonly ["up", "down"]
```

A `readonly` tuple is **not** assignable to a mutable `unknown[]`, so constraints that should accept both are written `T extends readonly unknown[]`.

### Variadic tuple patterns

Spreads work in tuple types — and `infer` can capture the pieces:

```ts
type First<T extends unknown[]> = T extends [infer F, ...unknown[]] ? F : never;
type Tail<T extends unknown[]>  = T extends [unknown, ...infer R] ? R : [];
type T1 = Tail<[1, 2, 3]>;  // [2, 3]
```

A rest element may sit at the **start** of a pattern, too.
