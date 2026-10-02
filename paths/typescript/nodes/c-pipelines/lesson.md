A shop's "top sellers" widget looked fine for months. Then someone noticed the **main product list** had started showing up sorted by sales too, and nobody had asked for that:

```ts
function topSellers(products: Product[]) {
  return products.sort((a, b) => b.sold - a.sold).slice(0, 3);
}
```

`sort` doesn't return a sorted copy. It sorts **the array you gave it**, in place, and hands the same array back. The widget had been quietly reshuffling everyone else's data. This lesson is about building results out of arrays without touching the input: `filter`, `map`, `sort` on a copy, and `reduce`.

### A pipeline is a chain of small steps

Most "report" code has the same shape: keep some items, put them in order, turn each one into something new.

```
products ──filter──► in stock ──sort──► by price ──map──► "Mug: 9.50"
```

Each step takes an array and returns a **new** array, so you can chain them:

```ts
const lines = products
  .filter((p) => p.stock > 0)       // keep
  .map((p) => `${p.name}: ${p.price.toFixed(2)}`); // transform
```

`filter` and `map` never change the original. That's why "don't modify the input" and "remove items" aren't in conflict: you don't remove anything from `products`, you build a new array that leaves some out.

### `sort` is the odd one out: copy first

`sort` and `reverse` **mutate**. Make a copy before you sort:

```ts
const cheapestFirst = [...products].sort((a, b) => a.price - b.price);
// or, in ES2023:
const cheapestFirst2 = products.toSorted((a, b) => a.price - b.price);
```

The comparator returns a negative number if `a` goes first, a positive one if `b` goes first, and `0` for a tie. `a.price - b.price` gives ascending order, and `b.price - a.price` gives descending. For strings, use `a.name.localeCompare(b.name)`. Ties keep their original order, because `sort` is stable.

### `readonly` arrays: let the compiler guard the input

You can say in the type that a function won't change its array:

```ts
function cheapest(products: readonly Product[]) {
  products.sort(...);        // ✗ Property 'sort' does not exist on type 'readonly Product[]'
  return [...products].sort(...); // ✓ the copy is a normal array
}
```

`readonly Product[]` (or `ReadonlyArray<Product>`) removes `push`, `pop`, `splice`, `sort` and index assignment from the type. `filter`, `map`, `slice` and `toSorted` are all still there, because they return new arrays. A caller can pass a normal `Product[]` to a `readonly` parameter, so it costs them nothing. It's only checked by the compiler, though. At runtime it's an ordinary array.

### `reduce`: many values in, one value out

`reduce` walks the array and carries an **accumulator** from item to item. You give it a function `(acc, item) => newAcc` and a starting value:

```ts
const total = orders.reduce((sum, o) => sum + o.amount, 0);
```

The accumulator doesn't have to be a number. It can be an object that collects several things at once. When it is, **type the starting value**, or TypeScript infers the type from that literal and gets it wrong:

```ts
interface Totals { count: number; revenue: number; biggest: number | null }

const totals = orders.reduce<Totals>(
  (acc, o) => ({
    count: acc.count + 1,
    revenue: acc.revenue + o.amount,
    biggest: acc.biggest === null ? o.amount : Math.max(acc.biggest, o.amount),
  }),
  { count: 0, revenue: 0, biggest: null },
);
```

Without `<Totals>`, the start value `{ ..., biggest: null }` is typed `biggest: null`, and assigning a number to it later is an error. You can also write `{ ... } as Totals` on the start value, but the type argument is clearer.

Always pass the starting value. Without it, `reduce` uses the first item as the start and **throws** on an empty array.

### Formatting numbers for people

`toFixed(n)` turns a number into a string with exactly `n` decimals, and it rounds:

```ts
(9.5).toFixed(2);    // "9.50"
(12.345).toFixed(1); // "12.3"
(2).toFixed(2);      // "2.00"
```

It returns a **string**, so format at the very end of the pipeline. Do the maths on numbers, then turn them into text for display. Two traps: `toFixed` uses floating-point, so `(1.005).toFixed(2)` gives `"1.00"`, not `"1.01"` (store money as cents if it matters). And `0.1 + 0.2` is `0.30000000000000004`, which is exactly why you format before showing it.

### Gotchas

- **`sort()` with no comparator sorts as strings.** `[10, 9, 1].sort()` gives `[1, 10, 9]`. Always pass `(a, b) => a - b` for numbers.
- **`for...in` over an array gives you indices as strings**, not items. Use `for...of`, or the array methods above.
- **`map` for transforming, `forEach` for side effects.** If you write `forEach` and `push` into a new array, you wanted `map`.
- **Don't chain forever.** Three or four steps read well. Past that, name the intermediate arrays.

### In the wild

- **Redux reducers** are named after `reduce`: `(state, action) => newState`, folded over every action your app has dispatched.
- **React** re-renders only when it sees a *new* array. Sorting state in place is the classic "my list didn't update" bug, which is why React code is full of `[...items].sort(...)`.
- **ES2023 added `toSorted`, `toReversed` and `toSpliced`** precisely because mutating `sort` kept biting people.
- **Every dashboard** you've seen ("top 5 customers this month, revenue to two decimals") is a filter → sort → slice → map pipeline with a reduce for the totals.
