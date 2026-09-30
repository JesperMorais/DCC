Networks hiccup. Write a **decorator factory** `retry` so any flaky function can be wrapped with one line:

```python
def retry[**P, R](
    times: int, on: tuple[type[Exception], ...] = (Exception,)
) -> Callable[[Callable[P, R]], Callable[P, R]]
```

- `@retry(times=3)` calls the function **up to 3 times in total**, stopping at the first call that doesn't raise, and returns its result.
- Only exceptions that are instances of a class in `on` are retried. Any other exception propagates **immediately**.
- If every attempt fails, re-raise the exception from the **last** attempt.
- Positional and keyword arguments are passed through unchanged on every attempt.
- The wrapped function keeps its identity: `__name__` and `__doc__` must be the original's (use `functools.wraps`).
- `retry(times=0)` (or any `times < 1`) raises `ValueError` straight away.

No sleeping between attempts is needed.

```python
@retry(times=3, on=(ConnectionError,))
def fetch_user(user_id: int) -> str:
    """Fetch a user from the API."""
    ...

fetch_user(42)        # retried on ConnectionError, up to 3 calls
fetch_user.__name__   # "fetch_user"
```
