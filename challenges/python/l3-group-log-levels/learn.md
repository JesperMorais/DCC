### Grouping with `defaultdict`

With a plain dict, adding to a group means checking first whether the group exists:

```python
by_colour: dict[str, list[str]] = {}
if colour not in by_colour:
    by_colour[colour] = []
by_colour[colour].append(item)
```

`collections.defaultdict` does the check for you. You give it a **factory** (here `list`), and it calls that factory the first time a missing key is read:

```python
from collections import defaultdict

by_colour: defaultdict[str, list[str]] = defaultdict(list)
by_colour["red"].append("apple")    # "red" didn't exist, so it gets []
by_colour["red"].append("cherry")
dict(by_colour)                     # {"red": ["apple", "cherry"]}
```

Like a normal dict, it remembers the order keys were first added. `dict(...)` turns it back into a plain dict before you hand it to other code.

### Splitting once with `partition`

```python
"a=b=c".partition("=")   # ("a", "=", "b=c")
"abc".partition("=")     # ("abc", "", "")   the separator wasn't found
```
