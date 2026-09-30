### Tuples: arrays with a fixed shape

A tuple type says exactly how many elements there are and what type each position has:

```ts
type Point = [number, number];
const p: Point = [3, 4];
const [x, y] = p;         // x: number, y: number

type Entry = [string, number];
const e: Entry = ["apples", 3];
```

TypeScript *infers* `number[]` for `[a, b]` unless you say otherwise, so functions that return tuples usually need an explicit return type. You can even name the positions for readability: `[x: number, y: number]`.

### `readonly` arrays

```ts
function average(values: readonly number[]): number {
  values.push(0);  // Error: push doesn't exist on readonly number[]
  values.sort();   // Error: sort would mutate
  return values.reduce((a, b) => a + b, 0) / values.length; // fine
}
```

`readonly` promises the caller "I won't change your array". Non-mutating methods like `map`, `filter`, `reduce`, `slice` still work.
