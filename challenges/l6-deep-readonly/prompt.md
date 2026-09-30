The built-in `Readonly<T>` is shallow: nested objects stay mutable. Write **`DeepReadonly<T>`**, which makes **every** property at **every** depth `readonly`.

```ts
type Config = {
  name: string;
  db: { host: string; ports: number[] };
};

type Frozen = DeepReadonly<Config>;
// {
//   readonly name: string;
//   readonly db: { readonly host: string; readonly ports: readonly number[] };
// }
```

- Arrays become `readonly` arrays, tuples become `readonly` tuples — and their elements are deep-readonly too.
- **Functions are left untouched** (they must stay callable).
- Primitives (`string`, `number`, …) are returned as-is.
