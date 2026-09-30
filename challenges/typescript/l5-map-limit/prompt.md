`Promise.all(items.map(fn))` fires **every** request at once — fine for 5 items, a disaster for 5 000 (rate limits, sockets, memory). Write **`mapLimit`**, which runs an async function over a list with at most `limit` calls in flight.

```ts
async function mapLimit<T, R>(
  items: T[],
  limit: number,
  fn: (item: T) => Promise<R>,
): Promise<R[]>
```

- Never have more than `limit` calls of `fn` running at the same time — but as soon as one finishes, start the next.
- Resolve with the results **in the same order as `items`**, regardless of which finished first.
- If any call rejects, `mapLimit` rejects with that error.
- An empty list resolves to `[]`. You can assume `limit >= 1`.

```ts
await mapLimit([1, 2, 3, 4], 2, async (n) => n * 10); // [10, 20, 30, 40], ≤ 2 running at once
```
