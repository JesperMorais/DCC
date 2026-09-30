### What `for` really does

```python
for x in thing:
    body(x)
```

is roughly:

```python
it = iter(thing)            # calls thing.__iter__()
while True:
    try:
        x = next(it)        # calls it.__next__()
    except StopIteration:
        break
    body(x)
```

So any object with the right two dunder methods works in `for`, `list()`, `sum()`, unpacking, `zip`…

### Two roles: iterable and iterator

- An **iterable** has `__iter__`, which returns an iterator. Lists, dicts and `range` are iterables, and you can loop over them again and again.
- An **iterator** has `__next__` (return the next value, or raise `StopIteration` when done) *and* an `__iter__` that returns `self`, so an iterator can also be used directly in a `for`. It is **single-use**: it holds the loop's position.

```python
r = range(3)
iter(r) is iter(r)   # False: a fresh cursor every time
it = iter(r)
iter(it) is it       # True
```

If one object plays both roles (its `__iter__` returns `self` and it stores the position), then two loops over it share one position, and the second `list(obj)` comes back empty. Keeping the **position** in a separate iterator object is what makes an iterable re-iterable.

### Typing `return self`

`from typing import Self` lets `def __iter__(self) -> Self:` say "returns this very object's type".
