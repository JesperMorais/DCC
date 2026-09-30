### Dunder methods hook into operators

Python turns operators into method calls: `a == b` calls `a.__eq__(b)`, `a < b` calls `a.__lt__(b)`, and `repr(a)` calls `a.__repr__()`. `sorted()` only needs `<`.

```python
class Money:
    def __init__(self, cents: int) -> None:
        self.cents = cents

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Money):
            return NotImplemented   # "I don't know how to compare with that"
        return self.cents == other.cents

    def __repr__(self) -> str:
        return f"Money({self.cents})"
```

Returning `NotImplemented` (not `False`) lets Python try the other operand, and then fall back to "not equal". Annotate `other` as `object`, because anything can end up on the right-hand side.

### `functools.total_ordering`

Write `__eq__` and `__lt__`, decorate the class with `@total_ordering`, and it fills in `<=`, `>` and `>=` for you.

### Alternative constructors with `@classmethod`

A classmethod receives the **class** as `cls` instead of an instance. It's the idiomatic way to offer a second way to build objects:

```python
@classmethod
def from_euros(cls, text: str) -> Self:   # from typing import Self
    return cls(round(float(text) * 100))
```

Using `cls(...)` rather than `Money(...)` means subclasses get the right type back.
