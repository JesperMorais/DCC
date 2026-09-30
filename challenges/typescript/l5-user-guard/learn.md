### `unknown` — the safe `any`

`any` switches the type checker off. `unknown` is the opposite: you can hold anything, but you can't *use* it until you prove what it is.

```ts
const data: unknown = JSON.parse(text);
data.name;            // ❌ error: 'data' is of type 'unknown'
```

### User-defined type guards

A function returning `x is T` is a **type guard**. When it returns `true`, TypeScript narrows the argument to `T` in that branch:

```ts
function isPoint(v: unknown): v is { x: number; y: number } {
  return typeof v === "object" && v !== null
    && typeof (v as Record<string, unknown>).x === "number"
    && typeof (v as Record<string, unknown>).y === "number";
}

if (isPoint(data)) data.x;   // ✅ data is { x: number; y: number } here
```

Careful: the compiler **trusts** your guard. If the body is wrong, you've just lied to the type system.

### Assertion functions

`asserts v is T` means "if this function returns at all, `v` is `T`". It narrows *everything after the call*, no `if` needed — perfect for "validate or crash" at the edge of your program:

```ts
assertIsPoint(data);
data.y;   // ✅
```
