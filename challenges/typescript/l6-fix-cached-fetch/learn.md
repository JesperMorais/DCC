### Sharing in-flight requests

A common trick for caches is to remember the **promise**, not just the value. If five components ask for the same user at once, the first `get` starts the request and stores its promise in a `Map`; the other four get that same promise. One network call, five happy callers.

The tricky part is *removing* that entry. A promise ends in one of two ways, and cleanup has to happen for both:

```ts
const p = load()
  .then((v) => { /* runs only on success */ return v; })
  .catch((e) => { /* runs only on failure */ throw e; })
  .finally(() => { /* runs either way, value or error passes through */ });
```

`then(onFulfilled)` with a single callback skips straight past a rejection; the rejection just flows on to whoever awaits the result. Code that only runs inside it never runs when things go wrong.

### Debugging technique: trace the unhappy path

Happy-path tests passing tells you little about error handling. Make a fake dependency that fails exactly once (`attempts++ === 0 ? throw : return`) and walk through the code for that call: at each step, which callbacks run, and what state is left behind when the call is over? State that outlives the failure is the usual suspect.
