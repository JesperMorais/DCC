**Bug report from the web shop team**

> The "Sort by price" button works. But after someone clicks it, the "Featured" tab also shows products cheapest first, and the merch team spent all week choosing that order.

`cheapestFirst(products)` returns the products ordered by `priceCents`, lowest first (products with the same price keep their original order). The array passed in is the shop's master catalog: it must come back **unchanged**.

```ts
const catalog: Product[] = [
  { name: "Notebook", priceCents: 1200 },
  { name: "Pen", priceCents: 250 },
  { name: "Backpack", priceCents: 4500 },
];

cheapestFirst(catalog).map((p) => p.name); // ["Pen", "Notebook", "Backpack"]
cheapestFirst([]);                          // []
```

**Reproduction:** after `cheapestFirst(catalog)`, `catalog[0].name` is `"Pen"`, expected `"Notebook"`.

Find the bug and fix it.
