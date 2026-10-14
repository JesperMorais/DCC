### `reduce` folds an array into one value

`reduce` walks an array and carries a running value, the **accumulator**, from one item to the next:

```ts
const lengths = ["hi", "hello"].map((w) => w.length); // [2, 5]
const total = lengths.reduce((acc, n) => acc + n, 0);  // 7
```

Your function is called once per element. Its return value becomes `acc` for the next call. The second argument (`0` above) is the **initial value**: what `acc` is before the first call, and what you get back when the array is empty.

You may leave the initial value out. Then `reduce` uses the first element as the starting `acc` and begins at the second. That's fine for a non-empty array, but with nothing in it there's nothing to start from.

### Debugging: read the error message

When code throws, the message usually names the operation that failed. Search the code for that operation (here, the method named in the message), then ask which inputs could put it in the state the message describes.
