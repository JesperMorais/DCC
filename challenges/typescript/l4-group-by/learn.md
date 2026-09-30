### `Map<K, V>`

A `Map` is a key → value store. Unlike a plain object, its keys can be any type, and it remembers insertion order.

```ts
const stock = new Map<string, number>();
stock.set("apples", 3);
stock.get("apples");  // 3
stock.get("pears");   // undefined  ← get() returns V | undefined
stock.has("pears");   // false
[...stock.keys()];    // ["apples"]
```

Because `get` may return `undefined`, TypeScript makes you handle the "not there yet" case before using the value.

### Generic callbacks

A type parameter can appear in a callback's signature. TypeScript infers it from what the callback *returns*:

```ts
function mapAll<T, R>(items: readonly T[], fn: (item: T) => R): R[] {
  return items.map(fn);
}

mapAll([1, 2], (n) => n > 1);        // R = boolean → boolean[]
mapAll(["a"], (s) => s.length);      // R = number  → number[]
```

The same trick lets a function's *return type* depend on a callback you pass in.
