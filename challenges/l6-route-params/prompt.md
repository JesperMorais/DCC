Web frameworks like Express let you write routes such as `"/users/:id"`. Make the compiler understand them: write **`Params<Path>`**, which turns a route string into an object type with one `string` property per `:param`.

```ts
type A = Params<"/users/:id">;                  // { id: string }
type B = Params<"/users/:id/posts/:postId">;    // { id: string; postId: string }
type C = Params<"/about">;                       // {}
```

- A param starts after `:` and runs until the next `/` or the end of the string.
- Static segments are ignored. No params → `{}`.
- `Params` only accepts string types — `Params<42>` must be a **type error**.

The tests also use your type in a `route(path, params)` signature, so a missing or misspelled param becomes a compile error — that's the payoff.
