Your API client must not send more than a few requests per second. Write **`make_rate_limiter`**, a factory that returns an `allow()` function with its own private state. No class allowed; use a **closure**.

```python
def make_rate_limiter(
    limit: int,
    window: float,
    clock: Callable[[], float] = time.monotonic,
) -> Callable[[], bool]
```

It's a **fixed-window** limiter:

- The first call to `allow()` opens a window at `clock()`.
- While `clock() - window_start < window`, the first `limit` calls return `True` and the rest return `False`.
- As soon as `clock() - window_start >= window`, the next call opens a **fresh** window at the current time, with a full budget.
- Every limiter made by `make_rate_limiter` is independent.

`clock` is injectable so tests can control time. Call it on every `allow()`, never at creation.

```python
now = 0.0
allow = make_rate_limiter(2, window=1.0, clock=lambda: now)
allow(), allow(), allow()   # True, True, False
now = 1.0
allow()                     # True  (new window opened at 1.0)
```
