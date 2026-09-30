Flaky network calls are a fact of life. Write **`retry`**, which calls an async function until it succeeds or you run out of attempts.

```ts
async function retry<T>(fn: () => Promise<T>, attempts: number): Promise<T>
```

- Call `fn()`. If its promise **resolves**, resolve `retry` with that value — don't call `fn` again.
- If it **rejects**, try again, up to `attempts` calls in total.
- If every attempt fails, `retry` must reject with the error from the **last** attempt.
- You can assume `attempts >= 1`. No delay between attempts is needed.

Examples:

- `fn` succeeds first time → `fn` called once, value returned.
- `fn` fails twice then returns `"ok"`, `attempts = 3` → resolves `"ok"` after 3 calls.
- `fn` always fails, `attempts = 2` → rejects with the 2nd error, `fn` called exactly twice.
