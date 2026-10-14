### Changing a list in place

Lists are **mutable**: methods like `.append()`, `.remove()` and `.pop()` change the list itself, and everyone holding that list sees the change.

```python
fruit = ["apple", "kiwi", "plum"]
fruit.remove("kiwi")     # removes the first "kiwi"
# fruit is now ["apple", "plum"]: "plum" moved from index 2 to index 1
```

Notice the last line: when an item is removed, **every item after it shifts one place to the left**.

### How a `for` loop walks a list

A `for` loop over a list keeps an internal position: first index 0, then 1, then 2, and so on, until it runs past the end. It doesn't know or care whether the list changed in between.

```python
for i, item in enumerate(["a", "b", "c"]):
    print(i, item)       # 0 a, 1 b, 2 c
```

### Debugging technique: print the state each step

Put `print(task, tasks)` as the first line inside the loop and run it on the failing example. Comparing which item the loop is looking at with what the list looks like at that moment tells you a lot.
