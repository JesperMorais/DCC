### Errors as values

`throw` is invisible in a function's type: `parse(s: string): number` doesn't tell you it might blow up. A **Result type** puts failure *into the signature*, so the caller can't forget to handle it.

It's just a **discriminated union** — a shared literal field tells the variants apart:

```ts
type Loaded<T> =
  | { status: "done"; data: T }
  | { status: "failed"; reason: string };

function show(x: Loaded<string>) {
  if (x.status === "done") x.data;     // narrowed: data exists
  else x.reason;                       // narrowed: reason exists
}
```

Accessing `x.data` *before* checking `status` is a compile error — that's the safety.

### `never` as "no value possible"

`never` is the empty type: nothing inhabits it, and it's assignable to **every** type. So a helper that can only ever succeed can say its error type is `never`:

```ts
const nothingWrong: Loaded<never> = { status: "failed", reason: "x" };
// a Result<number, never> can be passed where Result<number, string> is expected
```

That's what lets `return ok(5)` type-check inside a function declared to return `Result<number, string>`.
