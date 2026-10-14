### Sorting by several things at once

`sorted(items, key=f)` calls `f` on every item and sorts by the results. If `f` returns a **tuple**, tuples are compared element by element: the first elements decide, and only on a tie does Python look at the second, and so on.

```python
people = [("Ada", 36), ("Linus", 28), ("Grace", 36)]
sorted(people, key=lambda p: (p[1], p[0]))
# [("Linus", 28), ("Ada", 36), ("Grace", 36)]   age, then name
```

`reverse=True` flips the **whole** comparison, every element of the tuple, not just the first. To make one numeric part descending while others stay ascending, a common trick is to negate that number in the key: `-age` sorts high ages first.

### Debugging technique: print the keys

When a sort comes out in a surprising order, look at exactly what's being compared:

```python
for p in people:
    print(key(p))
```

Then sort those printed tuples in your head, in the direction the code asks for, and compare with what you wanted. Pay special attention to the cases where the first elements are equal: that's where the later elements, and their direction, take over.
