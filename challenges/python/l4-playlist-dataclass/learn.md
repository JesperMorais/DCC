### `@dataclass` writes the boring parts

List the fields with type hints, and the decorator generates `__init__`, `__repr__` and `__eq__` for you:

```python
from dataclasses import dataclass

@dataclass
class Point:
    x: int
    y: int = 0

Point(3)                 # Point(x=3, y=0)
Point(3) == Point(3, 0)  # True, compares field by field
```

You can still add ordinary methods that use `self`.

### Mutable defaults need a factory

`tags: list[str] = []` would hand **one shared list** to every instance, so `@dataclass` refuses it with a `ValueError`. Use `field(default_factory=...)`. The factory is called once **per instance**:

```python
from dataclasses import dataclass, field

@dataclass
class Order:
    id: int
    items: list[str] = field(default_factory=list)
```

### `divmod` and zero-padding

```python
divmod(125, 60)       # (2, 5)   quotient and remainder at once
f"{5:02d}"            # "05"
```
