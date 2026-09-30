A weather dashboard shows today's lowest and highest temperature. Write `minMax(temps)` that returns **both** values at once as a pair `[min, max]`.

- The return type must be a **tuple** `[number, number]` (or `undefined`) — not `number[]`. The tests check the type.
- If the list is empty, return `undefined`.
- The input is a `readonly` array: **don't modify it**.

```ts
minMax([12, -3, 7, 22, 5]); // [-3, 22]
minMax([4]);                // [4, 4]
minMax([]);                 // undefined

const range = minMax([12, -3, 7]);
if (range) {
  const [low, high] = range; // low: number, high: number
}
```
