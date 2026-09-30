### Defaults are evaluated once

Python evaluates a default value **when the `def` line runs**, a single time, and stores that object on the function. Every call that relies on the default gets **the same object**:

```python
def stamp(when: list[str] = []) -> list[str]:
    when.append("now")
    return when

stamp()  # ["now"]
stamp()  # ["now", "now"]   the same list again
```

That's harmless for immutable values like `0`, `""` or `None`. For a `list`, `dict` or `set` that the function **mutates**, state leaks from one call into the next.

### The `None` sentinel

The standard fix is to default to `None` and build the fresh object inside the function, where it runs on every call:

```python
def connect(options: dict[str, str] | None = None) -> None:
    if options is None:
        options = {}
    ...
```

Use `is None` rather than `if not options:`. An empty list passed in by the caller is still *their* list, and you should use it, not replace it.

You can inspect the stored defaults with `func.__defaults__`.
