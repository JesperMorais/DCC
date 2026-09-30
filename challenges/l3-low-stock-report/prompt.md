The warehouse team wants a daily list of products that need restocking.

Write `lowStockReport(products, threshold)` that returns an array of strings:

1. Keep only products whose `stock` is **below** `threshold` (strictly less).
2. Sort them by `stock`, lowest first. If two have the same stock, sort by `name` alphabetically.
3. Turn each into a line: `"<sku> <name> (<n> left)"`, or `"<sku> <name> (out of stock)"` when stock is `0`.

The input is `readonly` — don't reorder the caller's array.

```ts
const products: readonly Product[] = [
  { sku: "A-1", name: "Mug",    stock: 12 },
  { sku: "B-7", name: "Poster", stock: 2 },
  { sku: "C-3", name: "Cap",    stock: 0 },
  { sku: "D-4", name: "Badge",  stock: 2 },
];

lowStockReport(products, 5);
// ["C-3 Cap (out of stock)", "D-4 Badge (2 left)", "B-7 Poster (2 left)"]

lowStockReport(products, 0); // []
```
