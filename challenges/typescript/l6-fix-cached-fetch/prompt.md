**Bug report from on-call:** "The profile service had a 30-second outage this morning. It's been healthy for hours, but some users still can't open their profile page. The error is the same `503` from the outage. Reloading the page fixes it."

`createCachedFetcher(fetcher, ttlMs, now?)` wraps an async lookup with a small cache:

- `get(key)` returns a cached value if it's younger than `ttlMs`; otherwise it calls `fetcher(key)` and caches the result.
- While a fetch for a key is running, other `get`s for that key **share** it instead of starting a new one.
- If a fetch fails, everyone waiting for it gets the error, and **nothing is remembered**: the next `get` for that key tries again.
- `invalidate(key)` drops the cached value. `stats` counts cache hits and real fetches.

```ts
const users = createCachedFetcher((id: string) => api.loadUser(id), 5000);

await users.get("7");                         // fetches, stats.fetches === 1
await users.get("7");                         // cached, stats.hits === 1
await Promise.all([users.get("8"), users.get("8")]); // one fetch for both
```

**Reproduction** (the first `loadUser` call fails, later ones succeed):

```ts
await users.get("7"); // rejects with Error("503"), as expected
await users.get("7"); // rejects with Error("503") again, expected "user 7"
// and the fetcher was only called once
```
