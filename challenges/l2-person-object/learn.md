### Objects group related values

An **object** bundles several named values (called **properties**) together:

```ts
const book = { title: "Dune", pages: 412 };
book.title; // "Dune"   — read a property with a dot
book.pages; // 412
```

### Describing an object's shape

You can give an object shape a name with a type alias. Each property gets its own type:

```ts
type Book = {
  title: string;
  pages: number;
};

function isLong(b: Book): boolean {
  return b.pages > 300;
}

isLong({ title: "Dune", pages: 412 }); // true
isLong({ title: "Dune" });             // compile error: property 'pages' is missing
```

Now TypeScript knows exactly which properties exist, autocompletes them, and complains if one
is missing or has the wrong type.

### Returning a new object

Instead of changing an object you were given, you can build a fresh one:

```ts
function renamed(b: Book, title: string): Book {
  return { title: title, pages: b.pages };
}
```
