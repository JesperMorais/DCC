### Interfaces describe the shape of an object

```ts
interface Book {
  title: string;
  pages: number;
}

const b: Book = { title: "Dune", pages: 412 }; // OK
const c: Book = { title: "Dune" };             // Error: 'pages' is missing
```

Once a value has an interface type, the editor knows exactly which properties exist and what type each one has — typos like `b.pagse` are caught immediately.

### `reduce` folds an array into one value

```ts
const books: Book[] = [/* ... */];
const totalPages = books.reduce((acc, book) => acc + book.pages, 0);
```

`reduce` calls your function once per element. `acc` is the value returned by the previous call; the second argument (`0`) is where it starts. That starting value also matters for types: it tells TypeScript the accumulator is a `number`, and it makes an empty array return `0` instead of crashing.
