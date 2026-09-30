### `collections.Counter`

Counting things is so common that the standard library has a dict made for it:

```python
from collections import Counter

votes = Counter(["tea", "coffee", "tea", "water", "tea"])
votes["tea"]         # 3
votes["juice"]       # 0  (missing keys count as zero, no KeyError)
votes.most_common(2) # [("tea", 3), ("coffee", 1)]
```

`most_common(n)` returns `(item, count)` tuples, largest first. Items with equal counts come out in the order they were first counted.

### Feeding it a generator expression

You don't have to build a list first. A generator expression, which is a comprehension in parentheses, feeds items one at a time and can filter as it goes:

```python
words = ["Tea", "tea", "COFFEE", "x"]
Counter(w.lower() for w in words if len(w) > 1)
# Counter({"tea": 2, "coffee": 1})
```

### Tuple unpacking

```python
name, age, city = "Ada 36 London".split()
```

The number of names on the left must match the number of pieces. `_` is the usual name for a piece you don't need.
