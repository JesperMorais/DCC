**Bug report from the drawing app team:** "The *New shape* form won't compile any more. `createShape({ kind: \"circle\", radius: 4 })` is rejected with *'radius' does not exist in type …*, and the type tooltip for a draft shows only `kind` with no other fields."

The editor models shapes as a discriminated union, plus a few helper types (all in the starter):

- `ShapeKind<T>` – the union of `kind` strings.
- `OfKind<T, K>` – the member with that `kind`.
- `Without<T, K>` – `T` without the keys `K`. **For a union, it removes the keys from each member separately**, so the result is still a discriminated union.
- `Draft<T>` – a shape before it's saved: everything except `id`.
- `Patch<T>`, `Handlers<T, R>`, `Saved<T>` – used elsewhere in the editor.

```ts
type A = Draft<OfKind<Shape, "rect">>;
// { kind: "rect"; width: number; height: number }

type B = Draft<Shape>;
// | { kind: "circle"; radius: number }
// | { kind: "rect"; width: number; height: number }
// | { kind: "text"; content: string; fontSize?: number }

type C = Without<{ a: 1; b: 2 } | { a: 3; c: 4 }, "a">;
// { b: 2 } | { c: 4 }
```

**Reproduction:**

```ts
type B = Draft<Shape>;
// is { kind: "circle" | "rect" | "text" }, expected the three-member union above
```

This is a type-level fix: the tests are compile-time checks. Change only what's broken.
