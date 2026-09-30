### Many choices: `else if`

When there are more than two possibilities, you can chain checks with `else if`.
TypeScript tries them **from top to bottom** and runs only the **first** one that is true:

```ts
function sizeLabel(cm: number): string {
  if (cm >= 190) {
    return "tall";
  } else if (cm >= 160) {
    return "medium";
  } else {
    return "short";
  }
}

sizeLabel(200); // "tall"   — first check matched, the rest are skipped
sizeLabel(170); // "medium" — first check false, second true
```

Notice that the second check doesn't need to say "and less than 190" —
if we got that far, the first check already failed. **Order matters!**

### `return` stops the function

As soon as a function hits `return`, it's done. That means you can also write the
chain as separate `if`s that each `return`, without `else`.
