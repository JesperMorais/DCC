### Memoization and cache keys

A memoized function keeps a private `Map` (in a closure) from "these inputs" to "that answer". With one primitive argument the argument itself can be the key. With several arguments you need **one value that stands for all of them**, and two rules decide whether it works:

1. **Different inputs, different keys.** If two calls that should give different answers produce the same key, the second one gets the first one's answer. That's a *collision*.
2. **Equal inputs, equal keys.** Otherwise you never get a cache hit.

`Map` compares keys with `===`, so an array of arguments is useless as a key: every call creates a new array. A common trick is to turn the arguments into a string:

```ts
String(["a", 1]);          // "a,1"
JSON.stringify(["a", 1]);  // '["a",1]'
JSON.stringify([{ x: 1 }]);// '[{"x":1}]'
String([{ x: 1 }]);        // "[object Object]"  (every object looks the same)
```

### Debugging technique: print the key

When a cache returns the wrong thing, log the key next to the arguments for a few calls. If two different argument lists print the same key, you've found your collision.
