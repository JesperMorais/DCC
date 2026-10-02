You're building the till for a small coffee bar. Every order on the screen is an object, and the barista keeps getting orders for a "huge" latte that doesn't exist. Time to give orders a real shape.

**Step 1: the types.** Replace the two placeholders in the starter:

- `Size` is exactly one of `"small"`, `"medium"` or `"large"`. Any other string must be a type error.
- `CoffeeOrder` has four properties and nothing else:
  - `drink`: a string, like `"latte"`
  - `size`: a `Size`
  - `shots`: a number (how many espresso shots)
  - `oatMilk`: a boolean

**Step 2: three functions.**

`newOrder(drink, size)` returns a fresh order with one shot and regular milk:

```ts
newOrder("latte", "medium"); // { drink: "latte", size: "medium", shots: 1, oatMilk: false }
```

`orderPrice(order)` returns the price in kronor:

| | |
|---|---|
| small / medium / large | 30 / 35 / 40 |
| each shot **beyond the first** | +6 |
| oat milk | +5 |

```ts
orderPrice({ drink: "latte", size: "small", shots: 1, oatMilk: false });     // 30
orderPrice({ drink: "flat white", size: "large", shots: 3, oatMilk: true }); // 40 + 12 + 5 = 57
```

`withOatMilk(order)` returns a **new** order that is the same except `oatMilk` is `true`. The original order must not change, because it's still shown on the screen.
