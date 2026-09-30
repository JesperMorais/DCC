### Array pipelines

`filter`, `map` and friends each return a **new** array, so you can chain them into a readable pipeline — every step does one job:

```ts
interface Order { id: string; total: number; paid: boolean }

const unpaidIds = orders
  .filter((o) => !o.paid)          // Order[]
  .map((o) => o.id);               // string[]
```

TypeScript tracks the element type through every step, so after `.map((o) => o.id)` it knows you have a `string[]`.

### Sorting safely

`sort` is the odd one out: it sorts **in place** and returns the same array. On a `readonly` array it isn't even allowed. Sort a copy instead — `[...arr].sort(...)` — or sort an array that a previous step already created.

A comparator returns a negative number if `a` comes first, positive if `b` comes first, and `0` for a tie:

```ts
orders.slice().sort((a, b) => b.total - a.total); // biggest first
"apple".localeCompare("banana");                   // negative
```

Because `0` is falsy, `first || second` is a neat way to express "sort by X, then by Y".
