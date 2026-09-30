### `keyof` — the union of an object type's keys

```ts
interface Product { sku: string; price: number }
type ProductKey = keyof Product; // "sku" | "price"
```

### Indexed access types — `T[K]`

Square brackets work on **types** too, and give you the type of that property:

```ts
type Price = Product["price"];          // number
type Either = Product["sku" | "price"]; // string | number
```

### Putting them together with generics

A second type parameter can be constrained by the first one:

```ts
function getProp<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}

const p = { sku: "A-1", price: 9 };
getProp(p, "price"); // number
getProp(p, "sku");   // string
getProp(p, "size");  // Error: "size" is not assignable to "sku" | "price"
```

Because `K` is inferred as the *literal* key you passed (e.g. `"price"`), `T[K]` resolves to exactly that property's type.
