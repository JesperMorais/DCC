### `sorted` with a `key`

`sorted(items, key=f)` calls `f` on every item and sorts by the results. It returns a **new list** and leaves the original alone, unlike `list.sort()`, which changes the list in place.

```python
words = ["kiwi", "fig", "banana"]
sorted(words, key=len)          # ["fig", "kiwi", "banana"]
sorted(words, key=len, reverse=True)
```

### Tuple keys for tie-breaks

Tuples compare **item by item**: `(2, "b") < (2, "c") < (3, "a")`. So a key that returns a tuple sorts by the first field and uses the next field to break ties:

```python
people = [("mia", 30), ("al", 25), ("bo", 30)]
sorted(people, key=lambda p: (p[1], p[0]))
# [("al", 25), ("bo", 30), ("mia", 30)]  by age, then name
```

To flip just **one** numeric field, negate it inside the tuple (`-p[1]`). `reverse=True` would flip every field.

### `enumerate`

```python
for place, name in enumerate(["gold", "silver"], start=1):
    print(place, name)  # 1 gold, then 2 silver
```
