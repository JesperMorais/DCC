Make a 2D `Vector` for a small game engine that behaves like a **built-in value type**, with operators, `abs()`, truthiness and a readable `repr`.

```python
class Vector:
    x: float
    y: float
```

| Expression | Result |
|---|---|
| `Vector(1, 2) + Vector(3, 4)` | `Vector(4, 6)` |
| `Vector(1, 2) - Vector(3, 4)` | `Vector(-2, -2)` |
| `Vector(1, 2) * 3` **and** `3 * Vector(1, 2)` | `Vector(3, 6)` |
| `abs(Vector(3, 4))` | `5.0` (the length) |
| `bool(Vector(0, 0))` | `False`. Every other vector is truthy. |
| `Vector(1, 2) == Vector(1, 2)` | `True`. Comparing with a non-vector (e.g. `(1, 2)`) is simply `False`. |
| `repr(Vector(3, 4))` | `"Vector(3, 4)"` (each component shown with its own `repr`) |

Also:

- Vectors are **immutable** (`v.x = 5` raises `AttributeError`) and **hashable**, so equal vectors can be deduplicated in a `set`.
- Unsupported operands, like `Vector * Vector` or `Vector + 1`, must raise `TypeError`, the same way `[] * []` does. Let Python raise it for you.
