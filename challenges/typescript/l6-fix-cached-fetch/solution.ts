type CacheEntry<V> = { value: V; storedAt: number };

type CachedFetcher<K, V> = {
  get(key: K): Promise<V>;
  invalidate(key: K): void;
  readonly stats: { hits: number; fetches: number };
};

function createCachedFetcher<K, V>(
  fetcher: (key: K) => Promise<V>,
  ttlMs: number,
  now: () => number = () => Date.now(),
): CachedFetcher<K, V> {
  const cache = new Map<K, CacheEntry<V>>();
  const inflight = new Map<K, Promise<V>>();
  const stats = { hits: 0, fetches: 0 };

  function fresh(key: K): CacheEntry<V> | undefined {
    const entry = cache.get(key);
    if (entry && now() - entry.storedAt < ttlMs) return entry;
    cache.delete(key);
    return undefined;
  }

  function get(key: K): Promise<V> {
    const entry = fresh(key);
    if (entry) {
      stats.hits++;
      return Promise.resolve(entry.value);
    }

    const pending = inflight.get(key);
    if (pending) return pending;

    stats.fetches++;
    const request = fetcher(key).then((value) => {
      cache.set(key, { value, storedAt: now() });
      return value;
    }).finally(() => inflight.delete(key));
    inflight.set(key, request);
    return request;
  }

  function invalidate(key: K): void {
    cache.delete(key);
  }

  return { get, invalidate, stats };
}
