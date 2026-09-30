### Discriminated unions

Give every member of a union a shared literal property — the *discriminant* — and TypeScript can tell them apart with a simple comparison:

```ts
type Shape =
  | { kind: "circle"; radius: number }
  | { kind: "square"; side: number };

function area(s: Shape): number {
  switch (s.kind) {
    case "circle":
      return Math.PI * s.radius ** 2; // s is the circle variant here
    case "square":
      return s.side ** 2;             // s.radius would be an error here
  }
}
```

### Exhaustiveness with `never`

`never` is the type with no values. After every case has been handled, the variable's remaining type *is* `never` — so assigning it to a `never` variable compiles. If a new variant is added and forgotten, the leftover type is no longer `never`, and you get a compile error pointing at the missing case:

```ts
default: {
  const unhandled: never = s; // Error if some kind isn't covered
  throw new Error(`Unknown shape: ${JSON.stringify(unhandled)}`);
}
```

### Immutable updates

```ts
const next = items.map((it) => (it.id === id ? { ...it, done: true } : it));
```

`map` builds a new array; spread builds a new object for the one item that changed.
