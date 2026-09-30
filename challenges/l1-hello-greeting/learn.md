### Type annotations on functions

In TypeScript you write the type **after** the name, separated by a colon:

```ts
function double(n: number): number {
  return n * 2;
}
```

- `n: number` — the parameter must be a number
- `): number` — the function promises to return a number

If you try `double("5")`, TypeScript refuses to compile. That's the whole point:
mistakes are caught *before* the code runs.

### Template literals

```ts
const who = "world";
const msg = `Hello, ${who}!`; // "Hello, world!"
```
