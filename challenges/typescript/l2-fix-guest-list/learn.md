### Finding things with `indexOf`

`indexOf` searches an array and returns the **position** (index) of the first match. Positions start at `0`. If nothing matches, it returns `-1`:

```ts
const colors = ["red", "green", "blue"];
colors.indexOf("red");    // 0
colors.indexOf("blue");   // 2
colors.indexOf("pink");   // -1
```

### Truthy and falsy

An `if` doesn't need a real boolean. It accepts any value and treats some as "false-ish" (**falsy**): `0`, `""`, `null`, `undefined`, `NaN` and `false`. Everything else is **truthy**, including `-1`:

```ts
if (0)  { /* never runs */ }
if (-1) { /* runs! */ }
```

That's handy sometimes, but a number that means a **position** isn't a yes/no answer. Write the question you actually mean, using a comparison.

### Debugging: print intermediate values

When a condition behaves strangely, log the value it's testing: `console.log(guests.indexOf(name))`. Seeing the raw number for a few inputs often makes the problem obvious.
