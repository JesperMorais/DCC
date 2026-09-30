Your admin panel lists thousands of orders, so they're shown in **pages**. Write a generic `chunk` function that splits an array into groups of at most `size` elements.

- The last group may be shorter.
- An empty array gives `[]`.
- `size` is always a whole number ≥ 1.
- Don't modify the input (it's `readonly`).

The starter uses `any`, which loses the element type. Make `chunk` **generic** so that chunking a `string[]` gives a `string[][]`, chunking `Order[]` gives `Order[][]`, and so on — the tests check this.

```ts
chunk([1, 2, 3, 4, 5], 2);   // [[1, 2], [3, 4], [5]]
chunk(["a", "b", "c"], 5);   // [["a", "b", "c"]]
chunk([], 3);                // []
```
