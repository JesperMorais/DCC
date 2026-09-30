Re-implement the built-in `ReturnType<T>` as **`MyReturnType<T>`** — without using `ReturnType`.

```ts
type A = MyReturnType<() => string>;                  // string
type B = MyReturnType<(a: number, b: string) => 1 | 2>; // 1 | 2
type C = MyReturnType<typeof Math.random>;            // number
```

- It must work for functions with **any** parameters.
- Passing something that isn't a function type (like `string`) must be a **type error**.

Type-only challenge: you win when every line in the tests compiles.
