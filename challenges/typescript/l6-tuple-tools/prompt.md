Build three utility types for tuples.

- **`Length<T>`** — the length of a tuple as a number literal type.
- **`TupleToUnion<T>`** — a union of all element types.
- **`Last<T>`** — the type of the last element, or `never` for an empty tuple.

```ts
type L = Length<["a", "b", "c"]>;          // 3
type U = TupleToUnion<[1, "two", true]>;   // 1 | "two" | true
type Z = Last<[string, number, boolean]>;  // boolean
type E = Last<[]>;                          // never
```

All three must accept `readonly` tuples (like the type of an `as const` array), and must be a **type error** for non-array types such as `string`.
