**Bug report from the checkout team:** "Since we wrapped the currency converter in `memoizeArgs`, every price in the shop comes out in euros. A Swedish customer saw `100 EUR` where it should have said `1150 SEK`."

`memoizeArgs` wraps a function of **any number of arguments** and caches its results:

```ts
function memoizeArgs<A extends unknown[], R>(fn: (...args: A) => R): (...args: A) => R
```

- Arguments are plain data (numbers, strings, booleans, arrays, plain objects). Two calls hit the same cache entry when **all** their arguments are equal by value.
- A call with any different argument runs `fn` again.
- Falsy results (`0`, `""`, `undefined`) are cached too, and every `memoizeArgs(...)` has its own cache.

```ts
const convert = memoizeArgs((amount: number, currency: string) => amount * rates[currency]);
convert(100, "EUR"); // 100   (runs fn)
convert(100, "EUR"); // 100   (cached, fn not called)
convert(100, "SEK"); // 1150  (runs fn)
```

**Reproduction:**

```ts
convert(100, "EUR"); // 100
convert(100, "SEK"); // 100, expected 1150
```
