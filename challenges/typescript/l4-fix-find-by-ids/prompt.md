**Bug report from support:** "The invoice screen crashes with `Cannot read properties of undefined (reading 'email')` whenever an order mentions a customer that has since been deleted."

`findByIds` looks up records by id in a `Map` and returns the ones it finds, **in the order of `ids`**. Ids that aren't in the map are skipped. It is built on a small reusable type guard, `isPresent`, which should be `true` for any value that is neither `null` nor `undefined`.

```ts
const customers = new Map([
  ["c1", { name: "Ada", email: "ada@example.com" }],
  ["c2", { name: "Linus", email: "linus@example.com" }],
]);

findByIds(customers, ["c2", "c1"]);   // [Linus, Ada]
findByIds(customers, ["c1", "gone"]); // [Ada]
isPresent(0);                         // true (0 is a real value)
```

**Reproduction:**

```ts
findByIds(customers, ["c1", "gone"]); // [Ada, undefined], expected [Ada]
```
