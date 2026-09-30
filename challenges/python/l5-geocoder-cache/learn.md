### Memoisation with `functools.lru_cache`

`@lru_cache(maxsize=N)` wraps a function so that repeated calls with the same (hashable) arguments return a stored result instead of running the body again:

```python
from functools import lru_cache

@lru_cache(maxsize=2)
def square(n: int) -> int:
    print("computing", n)
    return n * n

square(3); square(3)       # prints once
square.cache_info()        # CacheInfo(hits=1, misses=1, maxsize=2, currsize=1)
```

- **LRU** = *least recently used*. When the cache is full, the entry that was touched longest ago is dropped. A hit moves an entry back to "most recent".
- `maxsize=None` (or `functools.cache`) means unbounded. That's fine for pure maths, but a memory leak for a long-running service.
- If the function **raises**, nothing is cached.

### Cache the right key

The cache key is exactly the arguments. `"Main St"` and `"main st "` are different keys. So normalise *before* the cached call, by keeping the cached function small and putting a thin wrapper in front of it.

### A private cache per factory call

Decorating a function defined **inside** a factory gives every factory call its own, separate cache. The cached inner function is a closure, so it can call whatever the factory received:

```python
def make_scaler(factor: int) -> Callable[[int], int]:
    @lru_cache(maxsize=128)
    def scaled(n: int) -> int:
        return n * factor
    return scaled
```
