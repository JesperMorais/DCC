### Sets

A `set` is an unordered collection with **no duplicates**. Checking `x in s` is fast, however big the set is.

```python
tags = set(["red", "blue", "red"])  # {"red", "blue"}
"red" in tags                       # True
```

### Set operations

Sets support the maths you'd draw with Venn diagrams:

```python
morning = {"ada", "linus", "grace"}
evening = {"grace", "guido"}

morning & evening   # {"grace"}                           in both (intersection)
morning | evening   # {"ada", "linus", "grace", "guido"}  in either (union)
morning - evening   # {"ada", "linus"}                    only in morning (difference)
morning ^ evening   # {"ada", "linus", "guido"}           in exactly one
```

### Getting a stable order back

Sets have no order you can rely on, so tests and reports shouldn't depend on it. `sorted()` accepts any iterable, sets included, and always returns a **list**:

```python
sorted({"pear", "apple"})  # ["apple", "pear"]
```
