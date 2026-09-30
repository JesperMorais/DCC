### Repeating with `for`

A `for` loop runs the same indented block once for every item it's given. `range` hands out a sequence of whole numbers:

```python
for i in range(3):
    print(i)        # prints 0, then 1, then 2
```

`range(start, stop)` begins at `start` and stops **just before** `stop`:

```python
list(range(1, 4))   # [1, 2, 3]   (no 4!)
list(range(2, 2))   # []          (nothing to do)
```

### Building a list

A **list** holds several values in order: `[10, 20, 30]`. A common pattern is to start with an empty list and grow it with `.append(...)`, which adds an item at the end:

```python
laps: list[str] = []
for lap in range(1, 4):
    laps.append(f"Lap {lap}")

laps   # ["Lap 1", "Lap 2", "Lap 3"]
```

The type hint `list[str]` says "a list of strings". An empty `[]` gives mypy no clue what will go in it, so it's good practice to annotate it.
