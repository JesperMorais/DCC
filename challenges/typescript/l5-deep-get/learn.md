### Recursion

A recursive function solves a problem by solving a **smaller version of the same problem**. Every recursive function needs:

1. a **base case** that answers directly, and
2. a **recursive step** that makes the input smaller and calls itself.

```ts
function sumDigits(s: string): number {
  if (s === "") return 0;                        // base case
  return Number(s[0]) + sumDigits(s.slice(1));   // smaller input
}
```

If the input never shrinks toward the base case, you get infinite recursion (a stack overflow).

### Walking `unknown` safely

You can't index into `unknown` — TypeScript demands proof first. The idiom:

```ts
function field(v: unknown, key: string): unknown {
  if (typeof v !== "object" || v === null) return undefined;
  return (v as Record<string, unknown>)[key];
}
```

Remember that `typeof null === "object"` (a famous JavaScript wart), so always exclude `null` explicitly. Arrays are objects too, and `arr["0"]` works just like `arr[0]` — property keys are strings.
