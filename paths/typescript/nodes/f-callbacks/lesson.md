Here's a line that has confused JavaScript developers for over a decade:

```ts
["1", "7", "11"].map(parseInt); // [1, NaN, 3]
```

It compiles, it runs, and it's wrong. By the end of this lesson you'll know exactly why, and you'll be writing the loops from the last few nodes in a single line each.

### Functions are values

In TypeScript a function is a value, just like `42` or `"Ada"`. You can store one in a variable, and you can pass one to another function:

```ts
const double = (n: number): number => n * 2;

double(21); // 42
```

That's an **arrow function**: parameters in parentheses, then `=>`, then the result. It's the same as `function double(n: number): number { return n * 2; }`, just shorter. There are two body styles:

```ts
const double = (n: number) => n * 2;          // expression body: the value is returned
const loud = (n: number) => {                  // block body: needs an explicit return
  console.log("doubling", n);
  return n * 2;
};
```

A function you hand to another function, so that *it* can call yours, is a **callback**.

### `map`: transform every item

You've written this loop many times by now:

```ts
const prices = [100, 250, 40];
const withVat: number[] = [];
for (const p of prices) {
  withVat.push(p * 1.25);
}
```

The only interesting part is `p * 1.25`. Everything else is boilerplate. `map` takes just the interesting part, as a callback, and does the rest:

```ts
const withVat = prices.map((p) => p * 1.25); // [125, 312.5, 50]
```

`map` calls your callback once per item, in order, and collects what it returns into a **new** array of the same length. `prices` is untouched. You didn't annotate `p`: TypeScript knows `prices` is a `number[]`, so it infers `p: number`.

### `filter`: keep some items

`filter` also calls your callback once per item. If the callback returns `true`, the item is kept; if it returns `false`, it's skipped:

```ts
const cheap = prices.filter((p) => p < 200); // [100, 40]
```

The result is a new array containing the **original items** (not `true`/`false`), possibly fewer of them. Again, `prices` itself doesn't change. This is the "remove items without modifying the input" you did by hand in *Arrays without mutation*, now in one line.

### Worked example: chaining

Because both return arrays, you can chain them. Read it top to bottom like a recipe:

```ts
type Product = { name: string; price: number; inStock: boolean };

const shelf: Product[] = [
  { name: "Mug", price: 90, inStock: true },
  { name: "Poster", price: 150, inStock: false },
  { name: "Cap", price: 120, inStock: true },
];

const labels = shelf
  .filter((p) => p.inStock)          // keep the 2 in-stock products
  .map((p) => `${p.name}: ${p.price} kr`); // turn each into a string

// ["Mug: 90 kr", "Cap: 120 kr"]
```

Filter first, then map: it's cheaper to transform fewer items, and the `map` callback still receives whole `Product`s.

### Writing your own function that takes a callback

You can accept a callback too. Its type is written like an arrow function, with a return type after `=>`:

```ts
function countWhere(products: Product[], test: (p: Product) => boolean): number {
  let count = 0;
  for (const p of products) {
    if (test(p)) count++;
  }
  return count;
}

countWhere(shelf, (p) => p.price > 100); // 2
countWhere(shelf, (p) => p.colour === "red");
// ✗ Property 'colour' does not exist on type 'Product'
```

`(p: Product) => boolean` reads "a function that takes a `Product` and returns a `boolean`." The caller decides *what* to test, and your function decides *how* to loop.

### Gotchas

- **Braces swallow the result.** `nums.map((n) => { n * 2 })` returns `[undefined, undefined, …]`, because a block body needs `return`. TypeScript infers the result as `void[]`, which is your clue.
- **Callbacks get more than one argument.** `map` and `filter` call your callback with `(item, index, array)`. `parseInt` takes `(text, radix)`, so `.map(parseInt)` calls `parseInt("7", 1)`, and base 1 is invalid, so you get `NaN`. Write the arrow yourself: `.map((s) => parseInt(s, 10))`.
- **`map` copies the array, not the objects.** `shelf.map((p) => { p.price = 0; return p; })` returns a new array, but it changed every original product. To change a field, return a new object.
- **Use `forEach` (or `for...of`) for side effects.** If you're not using the returned array, `map` is the wrong tool and it confuses readers.

### In the wild

- **React** renders lists with `map`: `todos.filter((t) => !t.done).map((t) => <li key={t.id}>{t.title}</li>)`. Almost every React component that shows a list has this line.
- **Event handlers are callbacks.** `button.addEventListener("click", () => save())` hands the browser a function to call later. So does `setTimeout(() => …, 1000)`.
- **Express and Node** register routes with callbacks: `app.get("/users", (req, res) => …)`.
- **Code review.** "Can this loop be a `filter`?" is one of the most common comments on pull requests in TypeScript codebases.
