### Structural typing with `Protocol`

Python has always been duck-typed: "if it has `.read()`, treat it like a file". A `Protocol` writes that duck down so that **mypy** can check it:

```python
from typing import Protocol

class SupportsClose(Protocol):
    def close(self) -> None: ...

def shutdown(things: list[SupportsClose]) -> None:
    for t in things:
        t.close()
```

Any class with a matching `close` method satisfies `SupportsClose`, with **no inheritance required**. That's *structural* typing (matching by shape), as opposed to *nominal* typing (matching by declared base class). It's ideal for code that you don't own.

Add `@runtime_checkable` and `isinstance(x, SupportsClose)` also works at runtime. It only checks that the attribute **exists**, not its signature.

### Why a bound `TypeVar` instead of just the protocol?

```python
def pick(items: Iterable[SupportsClose]) -> SupportsClose: ...
def pick[T: SupportsClose](items: Iterable[T]) -> T: ...
```

Both accept the same inputs, but only the second **remembers what you passed in**. Give it a `list[Socket]` and mypy knows you got a `Socket` back, with all its other methods. The first version erases that to "something closeable". `[T: Bound]` (Python 3.12+) means "any type `T`, as long as it satisfies `Bound`".

### Handy built-ins

`max(iterable, key=..., default=...)` picks the element with the largest key (the first one on ties) and returns `default` for empty input. `sum(...)` also works on generators.
