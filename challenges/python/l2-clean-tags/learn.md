### List comprehensions

Building a new list from an old one is so common that Python has a short way to write it. This loop:

```python
prices = [120, 0, 45, 300]
doubled: list[int] = []
for p in prices:
    doubled.append(p * 2)
```

can be written as one line:

```python
doubled = [p * 2 for p in prices]    # [240, 0, 90, 600]
```

Read it as "**`p * 2`**, for each `p` in `prices`".

### Filtering with `if`

Add an `if` at the end to keep only some items:

```python
paid = [p for p in prices if p > 0]           # [120, 45, 300]
big_doubled = [p * 2 for p in prices if p > 100]   # [240, 600]
```

The `if` decides **whether** an item is kept. The expression at the front decides **what** goes into the new list.

### Truthiness

In a condition, empty things count as false: `""`, `[]`, `0` and `None`. So `if name:` means "if `name` isn't an empty string".
