Build a launch `Countdown` that works in `for` loops by implementing the **iterator protocol by hand**, with no generators.

```python
class Countdown:                 # the iterable
    def __init__(self, start: int) -> None: ...
    def __iter__(self) -> CountdownIterator: ...

class CountdownIterator:         # the iterator
    remaining: int
    def __iter__(self) -> Self: ...
    def __next__(self) -> int: ...
```

- Iterating a `Countdown(n)` yields `n, n-1, …, 1`. `Countdown(0)` yields nothing. A negative start raises `ValueError`.
- A `Countdown` is **re-iterable**: every `iter(countdown)` returns a **fresh**, independent `CountdownIterator`, so looping twice (or nesting loops) works.
- A `CountdownIterator` is its own iterator (`iter(it) is it`). Its `remaining` attribute says how many values are still to come.
- Once exhausted, `next(it)` keeps raising `StopIteration`.

```python
c = Countdown(3)
list(c)             # [3, 2, 1]
list(c)             # [3, 2, 1]  (again)

it = iter(c)
next(it)            # 3
it.remaining        # 2
list(it)            # [2, 1]
```
