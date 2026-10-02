Opening a database connection is slow, so your API keeps a few open and lends them out: a **pool**. Build a generic one that works for any kind of resource.

**1. `PoolExhaustedError`** is a class that extends `Error`:

- `new PoolExhaustedError(3)` has the message `All 3 resources are in use`, the `name` `"PoolExhaustedError"`, and a read-only `capacity` field (`3`).

**2. `Pool<T>`** is a class:

| Member | Behaviour |
|---|---|
| `constructor(resources: readonly T[])` | The pool owns these resources. Don't modify the caller's array. |
| `available` (getter) | How many resources are free right now. |
| `acquire(): T` | Checks out the free resource that has waited longest: the initial resources in order, and a released resource goes to the back of the line. If none is free, throw a `PoolExhaustedError` with the pool's capacity (the number of resources it was created with). |
| `release(resource: T): void` | Returns a checked-out resource to the pool. If it isn't currently checked out (never acquired, or released twice), throw an `Error` with the message `Resource is not checked out`. |
| `use(fn)` | Acquires a resource, calls `fn(resource)` and returns what `fn` returns. The resource is released **even if `fn` throws** (the error still reaches the caller). Its return type is whatever `fn` returns. |

Keep all state in `#private` fields: code outside the class sees only the four members above, so `Object.keys(pool)` is `[]`.

```ts
const pool = new Pool(["conn-a", "conn-b"]);
pool.acquire();        // "conn-a"
pool.available;        // 1
pool.use((c) => c.length); // 6 (borrows "conn-b" and gives it back)
pool.release("conn-a");
pool.release("conn-a"); // throws "Resource is not checked out"
```
