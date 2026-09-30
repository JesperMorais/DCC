### Arrow functions

A short way to write a function is the **arrow function**:

```ts
const double = (n: number) => n * 2;
double(4); // 8
```

`(n) => n * 2` means "take `n`, give back `n * 2`". Arrow functions are perfect for passing
a little piece of logic *into* another function.

### `map`: transform every item

`map` calls your function on each item and collects the results in a **new** array of the same length:

```ts
const prices = [10, 20, 30];
const withTax = prices.map((p) => p * 1.25); // [12.5, 25, 37.5]
```

### `filter`: keep some items

`filter` calls your function on each item and keeps the item only if the function returns `true`:

```ts
const ages = [12, 30, 17, 45];
const adults = ages.filter((a) => a >= 18); // [30, 45]
```

Both return a new array and leave the original untouched. TypeScript even figures out the
parameter type for you: inside `prices.map(...)`, `p` is already known to be a `number`.
