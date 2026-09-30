TypeScript ships with a built-in `Pick<T, K>` utility type. Re-implement it as **`MyPick<T, K>`** — without using `Pick`.

`MyPick<T, K>` builds a new object type by taking only the keys `K` from `T`:

```ts
interface Todo { title: string; description: string; done: boolean }

type Preview = MyPick<Todo, "title" | "done">;
// { title: string; done: boolean }
```

Passing a key that doesn't exist on `T` must be a **type error**.

This is a *type-only* challenge: there's no runtime code. You win when the compiler is happy with every `Expect<...>` line in the tests.
