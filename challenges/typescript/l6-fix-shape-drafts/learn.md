### Mapped types over unions

`keyof` on a union gives only the keys that **all** members have, because those are the only keys that are safe to read on "one of these":

```ts
type Pet = { kind: "cat"; meows: boolean } | { kind: "dog"; barks: boolean };
type K = keyof Pet;   // "kind"
```

So a mapped type that iterates `keyof T` (like `Pick`, `Omit`, or your own `{ [P in keyof T]: … }` written over some other key set) sees the union as one blurred object and the per-member fields disappear.

### Making a type distribute

A conditional type on a **naked type parameter** runs once per union member and unions the results:

```ts
type Each<T> = T extends unknown ? { value: T } : never;
type E = Each<"a" | "b">;   // { value: "a" } | { value: "b" }
```

`T extends unknown` is always true; it's only there to split the union. Anything you do inside the true branch sees one member at a time.

Some homomorphic mapped types (`{ [P in keyof T]: … }` with `T` a type parameter) distribute on their own. Mapped types over a *computed* key set, such as `Exclude<keyof T, K>`, don't.

### Debugging technique: shrink the input

When a type comes out wrong, feed the helper chain a single member, then the full union, and hover each step. The first helper whose output changes between "one member" and "whole union" is where things collapse.
