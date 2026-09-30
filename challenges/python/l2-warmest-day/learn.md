### The "best so far" pattern

To find the biggest or smallest thing without `max()`/`min()`, walk through the items and keep track of the best one you've seen **so far**. Here's how to find the shortest name:

```python
names = ["Bartholomew", "Ada", "Linus"]
shortest = names[0]          # start with the first one
for name in names:
    if len(name) < len(shortest):
        shortest = name

shortest   # "Ada"
```

Starting from the **first item** (not from some made-up number like `0` or `999`) means the answer is always a real item from the list.

### Index and value together: `enumerate`

`enumerate` gives you each item's position along with the item:

```python
for i, name in enumerate(["Ada", "Linus"]):
    print(i, name)    # 0 Ada, then 1 Linus
```

### `None` means "no answer"

`None` is Python's value for "nothing here". A function that sometimes has no result declares it in the return type with `int | None`, which reads as "an int, or None". To check for an empty list, use `if not items:`, because an empty list counts as false.
