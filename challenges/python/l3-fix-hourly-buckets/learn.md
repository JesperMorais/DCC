### Names point at objects

In Python, a variable (or a slot in a list) doesn't hold a copy of a value. It **refers to** an object. Two names can refer to the **same** object:

```python
a = [1, 2]
b = a            # no copy: b is the same list as a
b.append(3)
a                # [1, 2, 3]
a is b           # True: one object, two names
```

This only matters for **mutable** objects like lists, dicts and sets, because those can change after they're created.

### Building nested lists

Making a grid or a list of buckets means making **several separate** inner lists. A comprehension evaluates its expression **once per item**, so each round creates a fresh object:

```python
rows = [[0, 0] for _ in range(3)]   # three separate lists
rows[0] is rows[1]                  # False
```

### Debugging technique: check identity with `is`

`==` asks "do these look the same?", and `is` asks "are these the same object?". When changing one thing unexpectedly changes another, print `x is y` (or `id(x)`, `id(y)`). If it's `True`, they're one object with two names.
