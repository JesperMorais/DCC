### `zip` walks lists side by side

```python
sizes = ["S", "M", "L"]
prices = [100, 120, 140]
list(zip(sizes, prices))  # [("S", 100), ("M", 120), ("L", 140)]
```

Each step gives you a tuple, which you can **unpack** straight into two names: `for size, price in zip(sizes, prices):`. If the lists differ in length, `zip` stops at the shorter one without complaining.

### Comprehensions

A comprehension builds a new list or dict in one expression. You can add an `if` to filter:

```python
[p * 2 for p in prices]                    # [200, 240, 280]
[p for p in prices if p > 110]             # [120, 140]
{s: p for s, p in zip(sizes, prices)}      # {"S": 100, "M": 120, "L": 140}
```

Comprehensions can be nested, for example a list comprehension whose items are dict comprehensions. Keep each one short enough to read at a glance. If a cleanup step is shared by every item, do it once beforehand.
