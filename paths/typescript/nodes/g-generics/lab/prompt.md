Your shop shows a "recently viewed" strip, and the same logic is needed for products, search terms and category ids. The starter's helpers use `any`, so every caller gets `any` back. Make them **generic** so the caller's types come back out.

**1. `pushRecent(list, item, max)`** returns a new list with `item` at the front.

- If `item` is already in the list (compared with `===`), it moves to the front instead of appearing twice.
- The result has at most `max` items. The oldest ones (at the end) drop off.
- Calling it with a `string[]` returns a `string[]`. Pushing an item of a different type, like a string onto a `number[]`, is a type error.

**2. `zip(as, bs)`** pairs up two lists by position: `[[as[0], bs[0]], [as[1], bs[1]], …]`.

- It stops at the end of the shorter list.
- The two lists can have different element types, and each pair is a tuple. `zip(numbers, strings)` returns `[number, string][]`.

Neither function may modify its input.

```ts
pushRecent(["mug", "tee", "cap"], "sock", 3); // ["sock", "mug", "tee"]
pushRecent(["mug", "tee", "cap"], "cap", 3);  // ["cap", "mug", "tee"]
zip([1, 2, 3], ["a", "b"]);                   // [[1, "a"], [2, "b"]]
```
