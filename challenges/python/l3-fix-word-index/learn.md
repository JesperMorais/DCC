### Reading a dict safely with `.get`

`d[key]` raises `KeyError` if the key is missing. `d.get(key, default)` returns the default instead. It **only reads**: the dict is not changed.

```python
stock = {"apples": 3}
stock.get("pears", 0)   # 0
stock                   # still {"apples": 3}
```

### Grouping into lists

To group values under a key you need "the list for this key, creating it if needed". There are a few ways to write that:

```python
groups: dict[str, list[int]] = {}

if "a" not in groups:          # 1. check first
    groups["a"] = []
groups["a"].append(1)

groups.setdefault("b", []).append(2)   # 2. setdefault stores the default, then returns it
```

`collections.defaultdict(list)` is a third option that does the same thing automatically.

### Debugging technique: inspect the result, not just the output

When a function returns the wrong thing, print the data structure it builds **at several points**, for example after each line of input. Seeing *when* it stops changing (or never starts) narrows the problem down to a few lines.
