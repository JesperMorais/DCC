### Promises start running immediately

A promise is not a "task you can start later" — calling an async function **starts it now**. So this starts all requests at once, no matter what you do with the array afterwards:

```ts
const promises = urls.map((u) => fetchJson(u)); // all in flight already
```

To limit concurrency you must control **when you call** `fn`, not when you await it.

### The worker-pool pattern

A clean way to think about it: create a few *workers*, each an async loop that processes one item at a time. Two workers → at most two things in flight.

```ts
async function worker(queue: string[]) {
  while (queue.length > 0) {
    const job = queue.shift()!;
    await process(job);          // this worker waits; others keep going
  }
}
await Promise.all([worker(q), worker(q)]);
```

### Why no locks?

JavaScript runs one piece of code at a time. Between two `await`s your code is never interrupted, so reading and bumping a shared counter (captured in a closure) is safe.

`Promise.all` resolves when all inputs resolve — and rejects as soon as **any** of them rejects.
