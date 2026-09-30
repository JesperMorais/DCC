### `with` guarantees cleanup

A context manager runs *setup* code when a `with` block starts and *teardown* code when it ends, **however** it ends: normally, via `return`, or by an exception. Files, locks and DB transactions all work this way.

### `@contextmanager`: a generator with one `yield`

`contextlib.contextmanager` turns a generator into a context manager. Everything before the `yield` is setup, the yielded value becomes the `as` target, and everything after is teardown:

```python
from contextlib import contextmanager

@contextmanager
def cd(path: str) -> Iterator[str]:
    old = os.getcwd()
    os.chdir(path)
    try:
        yield path             # the with-block runs here
    finally:
        os.chdir(old)          # runs even if the block raised
```

If the block raises, the exception is **re-thrown at the `yield`**. Without `try/finally`, the code after `yield` would simply never run. `finally` cleans up and then lets the exception keep propagating (only `except` without re-raising would swallow it).

### "Missing" is not the same as `None`

`d.get(key)` returns `None` both for a missing key and for a key whose value *is* `None`. When you must tell them apart, use a private **sentinel** object that nobody else can have stored:

```python
_MISSING = object()
old = d.get(key, _MISSING)
if old is _MISSING: ...
```
