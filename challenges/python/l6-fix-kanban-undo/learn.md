### Names, objects and copies

A Python variable is a **name tag on an object**, not a box holding a value. Containers hold tags too: a dict of lists holds references to list objects that live on their own.

A **copy** decides how many levels get duplicated:

```python
original = {"fruit": ["apple"], "veg": ["leek"]}

shallow = original.copy()           # also: dict(original), {**original}
deep = {k: list(v) for k, v in original.items()}   # or copy.deepcopy(original)

original["nuts"] = []               # top level: only `original` changes
original["fruit"].append("pear")    # inner list: shared!

shallow   # {"fruit": ["apple", "pear"], "veg": ["leek"]}
deep      # {"fruit": ["apple"], "veg": ["leek"]}
```

A shallow copy gives you a new outer container whose entries point at the **same** inner objects. Changes to the outer level stay separate, changes *inside* the inner objects show up in both.

### Debugging technique: check identity with `is`

When state you saved "changes by itself", compare the saved object with the live one using `is` (or `id()`), level by level:

```python
saved = history[-1]
print(saved is live)                        # the outer container
print(saved["fruit"] is live["fruit"])      # the inner ones
```

`True` anywhere you expected an independent copy means two names are pointing at one object, and any change through one is visible through the other.
