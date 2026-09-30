Your app merges customers from two APIs, and the same customer sometimes shows up twice. Write `dedupeById(items)` that removes duplicates by their `id`.

- Keep the **first** occurrence of each id, and keep the original order.
- Don't modify the input.
- It must work for **any** object type that has a numeric `id` — customers, products, orders — and return that same type (the tests check that a `Customer[]` comes back as a `Customer[]`, not `{ id: number }[]`).
- Passing objects **without** an `id` must be a type error.

```ts
dedupeById([
  { id: 1, name: "Ada" },
  { id: 2, name: "Linus" },
  { id: 1, name: "Ada (copy)" },
]);
// [{ id: 1, name: "Ada" }, { id: 2, name: "Linus" }]

dedupeById([]); // []
```
