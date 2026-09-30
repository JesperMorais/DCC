### Generic constraints with `extends`

A bare type parameter `T` could be *anything* — a number, a string, `null` — so TypeScript won't let you read properties from it. A constraint narrows what callers may pass, and in return lets you use what the constraint guarantees:

```ts
function longest<T extends { length: number }>(a: T, b: T): T {
  return a.length >= b.length ? a : b; // .length is allowed
}

longest("hi", "hello");   // T = string
longest([1, 2], [3]);     // T = number[]
longest(1, 2);            // Error: number has no 'length'
```

Notice the return type is still `T`, not `{ length: number }`. The caller gets back the full, precise type they passed in.

### `Set` — a collection of unique values

```ts
const seen = new Set<string>();
seen.add("a");
seen.add("a");
seen.size;        // 1
seen.has("a");    // true
```

`has` is fast (it doesn't scan like `array.includes`), which makes a `Set` the go-to tool for "have I seen this before?".
