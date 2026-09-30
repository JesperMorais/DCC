Two small types, one big idea.

**1. `MyExclude<T, U>`** — re-implement the built-in `Exclude`: remove from the union `T` every member that is assignable to `U`.

```ts
type A = MyExclude<"a" | "b" | "c", "a">;          // "b" | "c"
type B = MyExclude<string | number | boolean, number | boolean>; // string
type C = MyExclude<"a", "a">;                       // never
```

**2. `IsNever<T>`** — `true` if `T` is exactly `never`, otherwise `false`.

```ts
type D = IsNever<never>;      // true
type E = IsNever<undefined>;  // false
type F = IsNever<string | never>; // false  (that's just string)
```

Don't use the built-in `Exclude`. Watch out — the "obvious" `IsNever` does not work, and figuring out *why* is the point.
