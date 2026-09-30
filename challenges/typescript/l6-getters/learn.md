### Template literal types

Backtick strings work at the type level too, and they **distribute over unions**:

```ts
type Event = `on${"Click" | "Hover"}`;  // "onClick" | "onHover"
type Up = Uppercase<"id">;              // "ID"
type Cap = Capitalize<"userName">;      // "UserName"
```

`Uppercase`, `Lowercase`, `Capitalize` and `Uncapitalize` are built-in "intrinsic" string types.

### Key remapping with `as`

A mapped type normally keeps each key's name. Adding `as` lets you compute a **new** key from the old one:

```ts
type Prefixed<T> = {
  [K in keyof T as `x_${K & string}`]: T[K];
};
type P = Prefixed<{ a: 1; b: 2 }>;  // { x_a: 1; x_b: 2 }
```

Two tricks hide in there:

- **`K & string`** — `keyof T` can be `string | number | symbol`. Intersecting with `string` keeps only the string keys; `0 & string` and `symbol & string` are `never`.
- **Remapping to `never` removes the key.** That's the idiomatic way to *filter* properties out of a mapped type.

Note the value side still uses the **original** key `K` — only the name changes.
