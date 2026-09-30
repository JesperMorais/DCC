### Conditional types distribute over unions

When the checked type is a **naked type parameter**, a conditional type is applied to *each member* of a union separately, and the results are unioned back together:

```ts
type Boxed<T> = T extends any ? { value: T } : never;

type X = Boxed<string | number>;
// = Boxed<string> | Boxed<number>
// = { value: string } | { value: number }
```

Without distribution you'd get `{ value: string | number }` — a different type!

### `never` is the empty union

`never` behaves like a union with zero members. Two consequences:

- `A | never` is just `A` — so returning `never` from a branch **deletes** that member.
- Distributing over `never` means "run for each of zero members" → the result is `never`, whatever the branches say.

### Switching distribution off

Distribution only happens when `T` stands alone on the left of `extends`. Wrap it in anything and TypeScript compares the type as a whole:

```ts
type NoDistribute<T> = [T] extends [string] ? "all strings" : "not all";
type Y = NoDistribute<"a" | 1>;   // "not all"
```
