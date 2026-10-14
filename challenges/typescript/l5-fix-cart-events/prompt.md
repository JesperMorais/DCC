**Bug report from the web shop:** "Clicking *Add to cart* throws `TypeError: Cannot read properties of undefined (reading 'count')` and the little number on the cart icon never changes. Emptying the cart works fine."

The shop uses a small typed `EventBus`. A `CartBadge` subscribes to it when it's created and keeps `count`, the number of items in the cart:

- `"added"` with `{ sku, qty }` increases `count` by `qty`.
- `"cleared"` sets `count` back to `0`.
- Several badges (header and sidebar) can listen to the same bus; each keeps its own count.

```ts
const bus = new EventBus<ShopEvents>();
const badge = new CartBadge(bus);

bus.emit("added", { sku: "mug", qty: 2 });   // badge.count === 2
bus.emit("added", { sku: "tea", qty: 1 });   // badge.count === 3
bus.emit("cleared", undefined);              // badge.count === 0
```

**Reproduction:**

```ts
bus.emit("added", { sku: "mug", qty: 2 });
// throws TypeError, expected badge.count to be 2
```
