Write **`memoize`**: it takes a one-argument function and returns a new function that caches results.

```ts
function memoize<A, R>(fn: (arg: A) => R): (arg: A) => R
```

- The first call with a given argument runs `fn` and stores the result.
- Later calls with the **same** argument (compared like `Map` keys, i.e. `===`) return the stored result **without** calling `fn` again.
- Falsy results (`0`, `""`, `false`, `undefined`) must be cached too.
- Two separate `memoize(...)` calls must **not** share a cache.

```ts
let calls = 0;
const slowSquare = (n: number) => { calls++; return n * n; };
const fast = memoize(slowSquare);
fast(4); // 16, calls === 1
fast(4); // 16, calls still 1
fast(5); // 25, calls === 2
```
