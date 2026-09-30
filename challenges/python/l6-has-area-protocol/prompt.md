A floor-planning tool gets shapes from three different libraries. None of them share a base class, but they all have an `area()` method. Describe that **shape of an object** with a `Protocol`, then write two helpers that accept anything matching it.

```python
@runtime_checkable
class HasArea(Protocol):
    def area(self) -> float: ...

def largest[T: HasArea](shapes: Iterable[T]) -> T | None
def total_area(shapes: Iterable[HasArea]) -> float
```

- `HasArea` is a **runtime-checkable protocol**: `isinstance(obj, HasArea)` is `True` for any object with an `area` method, whether or not its class inherits from anything.
- `largest` returns the **same object** that has the biggest area (not a copy, and not the area). On a tie, return the first. With no shapes, return `None`.
- `total_area` returns the sum of all areas, and `0.0` for no shapes.
- Both accept any iterable, including a one-shot generator.

```python
rooms = [Rect(3, 4), Circle(2), Rect(5, 1)]
largest(rooms)            # Circle(2): area ≈ 12.57 beats 12
total_area(rooms)         # ≈ 29.57
isinstance(Rect(1, 1), HasArea)   # True, with no inheritance needed
isinstance("kitchen", HasArea)    # False
```
