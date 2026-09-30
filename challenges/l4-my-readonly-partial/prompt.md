`Readonly<T>` and `Partial<T>` are two of the most-used utility types. Re-implement them yourself — **without** using the built-ins:

- **`MyReadonly<T>`** — same properties as `T`, but every one is `readonly` (think: a config object loaded once at startup that nobody may change).
- **`MyPartial<T>`** — same properties as `T`, but every one is optional (think: a half-filled form draft).

```ts
interface Todo { title: string; done: boolean }

type Frozen = MyReadonly<Todo>; // { readonly title: string; readonly done: boolean }
type Draft  = MyPartial<Todo>;  // { title?: string; done?: boolean }

declare const frozen: Frozen;
frozen.title = "x";             // ✗ compile error

const draft: Draft = {};        // ✓ fine
```

This is a *type-only* challenge: you win when the compiler is happy with every line in the tests.
