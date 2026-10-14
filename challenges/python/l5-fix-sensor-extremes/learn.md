### Iterables vs iterators

A **list** is an *iterable*: every time you loop over it, you get a fresh pass from the start. An **iterator** is a one-way stream: it remembers where it is, and once it's at the end it stays there.

```python
nums = [3, 1, 2]
it = iter(nums)
print(list(it))   # [3, 1, 2]
print(list(it))   # []  the stream is used up
```

Generators, `map`, `filter`, `zip`, `open(...)` file handles and generator expressions all produce iterators. They're lazy: values are computed only as someone asks for them, which saves memory. The price is that you can consume them **once**. Anything that loops (`for`, `sum`, `min`, `max`, `list`, `in`…) consumes them.

Note that an exhausted iterator doesn't raise an error. It just looks **empty**, and functions like `max(x, default=None)` happily accept an empty input.

### Debugging technique: print intermediate values

When a result is missing, print each intermediate value right before it's used, and ask what *type* it is:

```python
print(type(values), values)
```

If the printed object is an iterator, wrap it in `list(...)` *for printing only* in a scratch copy, and check whether a later step still sees the same items.
