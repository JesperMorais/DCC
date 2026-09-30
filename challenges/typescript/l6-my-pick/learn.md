### Mapped types

A mapped type loops over a union of keys and produces an object type:

```ts
type Flags = { [K in "a" | "b"]: boolean };
// { a: boolean; b: boolean }
```

### `keyof` and indexed access

```ts
interface User { id: number; name: string }
type Keys = keyof User;      // "id" | "name"
type Name = User["name"];    // string
```

### Generic constraints

`K extends keyof T` means "K can only be keys that exist on T". Using a key outside that set becomes a compile error — exactly how the built-in `Pick` behaves.

### Type tests

`Expect<Equal<A, B>>` only compiles when `A` and `B` are exactly the same type. `// @ts-expect-error` marks a line that *must* fail to compile.
