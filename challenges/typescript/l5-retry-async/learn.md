### `async`/`await` and errors

An `async` function always returns a `Promise`. Inside it, `await` pauses until a promise settles:

- a **resolved** promise gives you its value,
- a **rejected** promise is *thrown* at the `await` line.

That means ordinary `try/catch` works for async errors:

```ts
async function loadName(): Promise<string> {
  try {
    const user = await fetchUser();   // may reject
    return user.name;
  } catch (e) {
    return "anonymous";                // rejection lands here
  }
}
```

And `throw` inside an `async` function **rejects** the promise it returns.

### Typing the generic

`fn: () => Promise<T>` says "a function returning a promise of *something*". Because `retry` returns `Promise<T>` with the same `T`, callers keep full type information: retrying a `() => Promise<User>` gives back a `Promise<User>`.

### `catch (e)` is `unknown`

Under `strict`, the caught value is `unknown` — anything can be thrown, not just `Error`s. Store it as `unknown` and rethrow it as-is.
