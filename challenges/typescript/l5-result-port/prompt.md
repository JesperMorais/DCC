Instead of throwing, many codebases return a **`Result`**: either a success carrying a value, or a failure carrying an error. Build one.

1. **`type Result<T, E>`** — exactly `{ ok: true; value: T } | { ok: false; error: E }`.
2. **`ok(value)`** → `{ ok: true, value }`, typed `Result<T, never>`.
3. **`err(error)`** → `{ ok: false, error }`, typed `Result<never, E>`.
4. **`parsePort(input: string): Result<number, string>`**
   - Ignore surrounding whitespace.
   - If what's left isn't made only of digits `0-9` (at least one) → `err("not a number")`.
   - If the number is outside `1`–`65535` → `err("out of range")`.
   - Otherwise → `ok(port)`.
5. **`unwrapOr(result, fallback)`** — the value if `ok`, otherwise `fallback`.

```ts
parsePort("8080");    // { ok: true, value: 8080 }
parsePort(" 443 ");   // { ok: true, value: 443 }
parsePort("12.5");    // { ok: false, error: "not a number" }
parsePort("70000");   // { ok: false, error: "out of range" }
unwrapOr(parsePort("nope"), 3000); // 3000
```
