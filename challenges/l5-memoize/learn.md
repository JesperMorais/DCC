### Closures

A function *closes over* the variables that were in scope where it was **created** — and keeps them alive for as long as the function exists:

```ts
function makeCounter() {
  let count = 0;            // private to this counter
  return () => ++count;     // the arrow "remembers" count
}
const a = makeCounter();
const b = makeCounter();
a(); a();   // 2
b();        // 1 — b has its own count
```

Nobody outside can touch `count`. That's how you get private state without a class.

### Higher-order functions

A **higher-order function** takes and/or returns functions. Wrapping a function to add behaviour — logging, caching, retrying — is the classic use. With generics, the wrapper can promise to return *exactly the same shape* it was given:

```ts
function logged<A, R>(fn: (arg: A) => R): (arg: A) => R {
  return (arg) => { console.log(arg); return fn(arg); };
}
```

TypeScript infers `A` and `R` from whatever you pass in, so `logged(Math.sqrt)` is `(arg: number) => number`.

### `has` vs `get`

`map.get(key)` returns `undefined` both when the key is missing *and* when the stored value is `undefined`. Only `map.has(key)` tells the two apart.
