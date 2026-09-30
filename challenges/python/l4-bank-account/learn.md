### Classes bundle data and behaviour

```python
class Counter:
    def __init__(self, start: int = 0) -> None:
        self._count = start          # instance attribute

    def increment(self) -> int:
        self._count += 1
        return self._count
```

`__init__` runs when you create an object (`Counter()`), and `self` is that object. By convention, a leading `_` marks an attribute as internal, so callers shouldn't touch it directly.

### Exceptions signal "I refuse"

```python
if age < 0:
    raise ValueError("age can't be negative")
```

The caller can catch it with `try/except ValueError`. A good habit is to **validate first, then change state**. That way a rejected call leaves the object exactly as it was.

### Read-only properties

```python
class Circle:
    def __init__(self, r: float) -> None:
        self._r = r

    @property
    def radius(self) -> float:
        return self._r
```

`c.radius` reads like an attribute, but without a `@radius.setter` Python raises `AttributeError` if you try to assign to it.
