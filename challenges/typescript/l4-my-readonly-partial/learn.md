### Mapped types

A mapped type builds an object type by looping over a union of keys:

```ts
type Flags = { [K in "dark" | "compact"]: boolean };
// { dark: boolean; compact: boolean }
```

Loop over `keyof T` and look up each value with `T[K]`, and you get a *copy* of `T`:

```ts
type Copy<T> = { [K in keyof T]: T[K] };
```

On its own that's useless — but you can change things on the way through. For example, change every value type:

```ts
type Stringify<T> = { [K in keyof T]: string };
type Nullable<T>  = { [K in keyof T]: T[K] | null };
```

### Property modifiers

Object properties can carry two modifiers:

```ts
interface Example {
  readonly id: number; // can't be reassigned
  nickname?: string;   // may be missing
}
```

Mapped types can **add** these modifiers to every property they produce. Where would each one go in `{ [K in keyof T]: T[K] }`? Look at where they sit in the interface above.
