### Operators are method calls

`a + b` is `a.__add__(b)`, `abs(a)` is `a.__abs__()`, `if a:` calls `a.__bool__()`, and `repr(a)` calls `a.__repr__()`. Define the dunder and your class joins the syntax.

### `NotImplemented`: "not my job, ask the other side"

For `a * b`, Python first tries `a.__mul__(b)`. If that **returns** the special value `NotImplemented`, Python tries the *reflected* method `b.__rmul__(a)`. Only if both decline does it raise `TypeError`:

```python
class Money:
    def __init__(self, cents: int) -> None:
        self.cents = cents

    def __add__(self, other: "Money") -> "Money":
        if not isinstance(other, Money):
            return NotImplemented          # return, don't raise!
        return Money(self.cents + other.cents)

    def __radd__(self, other: int) -> "Money":
        # sum([m1, m2]) starts with 0 + m1, so accept 0
        return self if other == 0 else NotImplemented
```

(The class name is quoted because, before Python 3.14, it doesn't exist yet while its own body is running. mypy reads the quoted name as a normal type.)

That's how `3 * v` can work: `int.__mul__` doesn't know vectors, so it returns `NotImplemented`, and then **your** `__rmul__` gets a turn.

`__eq__` works the same way. Returning `NotImplemented` for foreign types makes `==` fall back to identity, so it yields `False` instead of crashing.

### Let `dataclass` write the boring parts

```python
@dataclass(frozen=True)
class Point:
    x: float
    y: float
```

This generates `__init__`, a field-wise `__eq__`, a matching `__hash__`, and blocks attribute assignment. If you write your own `__repr__` in the class body, `dataclass` keeps yours.

Rule of thumb: objects that are **equal must hash equal**, and anything hashable should be immutable.
