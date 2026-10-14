### What `await` actually waits for

`await` pauses **the `async` function it's written in**, and nothing else. The caller keeps going unless it awaits too.

```ts
async function step(n: number) {
  await pause();
  console.log("finished", n);
}

function run() {
  step(1);               // starts, then run() carries on
  console.log("run done");
}
// "run done" prints before "finished 1"
```

### Callbacks and promises

Array helpers like `map`, `filter` and `forEach` call your callback synchronously and don't know about promises. An `async` callback returns a `Promise` immediately; what the helper does with that promise is up to the helper. `map` hands you the promises (so you can `await Promise.all(...)` them). Other helpers throw the promise away.

A plain `for...of` loop is different: it's part of your own function body, so an `await` inside it pauses the loop itself.

### Debugging technique: log the order of events

Async bugs are about *when* things happen. Put numbered `console.log`s at the start of the function, around each `await`, and just before `return`, then read the output order. If "return" prints before the work it's supposed to report on, something isn't being waited for.
