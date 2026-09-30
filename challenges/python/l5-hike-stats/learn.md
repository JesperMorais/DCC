### `itertools.pairwise`: look at neighbours

Many problems are about *changes* between consecutive items, not the items themselves. `pairwise` yields overlapping pairs, with no `range(len(...) - 1)` and no off-by-one bugs:

```python
from itertools import pairwise

prices = [10, 12, 11, 15]
[b - a for a, b in pairwise(prices)]   # [2, -1, 4]
list(pairwise([7]))                    # []: fewer than two items, no pairs
```

### `itertools.groupby`: runs of equal keys

`groupby(items, key=f)` splits a sequence into **consecutive** runs where `f(item)` stays the same. It yields `(key, group)` pairs, and `group` is a lazy iterator over that run:

```python
from itertools import groupby

temps = [3, 5, -1, -4, 2, 8, 9]
[(frozen, list(run)) for frozen, run in groupby(temps, key=lambda t: t < 0)]
# [(False, [3, 5]), (True, [-1, -4]), (False, [2, 8, 9])]
```

Unlike SQL's `GROUP BY`, it **does not sort**. `False` shows up twice above because those runs aren't adjacent. That makes it perfect for "streaks" and "runs", which is exactly what you want here.

Consume each `group` before moving to the next one: advancing `groupby` invalidates the previous group.

### `max` on possibly-empty input

`max(values, default=0)` returns `0` instead of raising `ValueError` when `values` is empty.
