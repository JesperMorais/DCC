### Generators: functions that pause

Put `yield` in a function and calling it no longer runs the body. It returns a **generator**, and each `next()` runs the body up to the next `yield`, hands out that value, and **freezes** there until you ask again:

```python
def countdown(n: int) -> Iterator[int]:
    print("starting")
    while n > 0:
        yield n
        n -= 1

g = countdown(3)   # nothing printed yet!
next(g)            # prints "starting", returns 3
list(g)            # [2, 1]; when the body ends, iteration stops
```

Because values are produced on demand, a generator can describe an endless sequence. The *consumer* decides how much to take.

### Iterables vs iterators

A list is **iterable**: every `for` loop over it starts from the beginning. `iter(x)` gives you an **iterator**, a one-way cursor that remembers its position:

```python
it = iter([1, 2, 3, 4])
next(it)        # 1
list(it)        # [2, 3, 4], carrying on from where it was
```

If you need to keep pulling from the *same* stream across several steps, grab one iterator up front and keep using it.

### `itertools.islice`

`islice(it, k)` lazily takes at most `k` items from an iterator. It stops early if the iterator runs dry, and it never reads ahead.

### Beware `list(...)` on unknown input

`list(iterable)` reads **everything**. On an infinite stream it never returns.
