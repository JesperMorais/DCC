### How sets and dicts find things

A `set` or `dict` doesn't compare your key against every stored key. That would be slow. Instead it:

1. calls `hash(key)` to pick a **bucket**,
2. and only then uses `==` to compare against the few keys in that bucket.

So two objects are "the same key" only if they land in the same bucket **and** compare equal. That gives the one rule every custom class must follow:

> If `a == b`, then `hash(a) == hash(b)`.

(The reverse doesn't have to hold: different objects may share a hash, that's just a collision.)

```python
class Point:
    def __init__(self, x: int, y: int) -> None:
        self.x, self.y = x, y
    def __eq__(self, other: object) -> bool:
        return isinstance(other, Point) and (self.x, self.y) == (other.x, other.y)
    def __hash__(self) -> int:
        return hash((self.x, self.y))      # built from the same data __eq__ uses
```

If a class defines `__eq__` and no `__hash__`, Python makes its instances unhashable, precisely to stop the rule from being broken by accident.

### Debugging technique: check the invariant directly

Take two values that should be equal and check every half of the contract: `a == b`, `hash(a) == hash(b)`, `len({a, b})`. The first `False` tells you where to look.
