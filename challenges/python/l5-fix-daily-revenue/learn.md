### `itertools.groupby` groups *runs*

`groupby(items, key=f)` walks through `items` once and yields `(key, group)` pairs. A new group starts **every time the key changes** from one item to the next. It never looks back or ahead:

```python
from itertools import groupby

for k, g in groupby("aaabba"):
    print(k, list(g))
# a ['a', 'a', 'a']
# b ['b', 'b']
# a ['a']          a second "a" group!
```

It's designed for streams, so it doesn't buffer the whole input. That's what makes it fast and memory-friendly, and it's also why it only sees *consecutive* equal keys.

Combine that with a dict comprehension and keep in mind: when the same key appears twice, the **later** entry silently replaces the earlier one.

The alternative that doesn't care about order is a dict you accumulate into, such as `collections.defaultdict(int)`.

### Debugging technique: shrink the failing input

Remove items from the reproduction one at a time while it still fails. Smaller inputs make the pattern obvious: here, try two orders on the same day, then the same two orders with one from a different day **between** them. Which arrangement breaks?
