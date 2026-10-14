### Variables that live across a loop

A loop body runs again and again. Any line **inside** the body runs on **every** pass, including assignments:

```python
for word in ["a", "b", "c"]:
    seen = []          # a brand-new empty list on every pass
    seen.append(word)
# seen is ["c"]: the earlier words were thrown away
```

Values that need to **build up** over the whole loop (a total, a count, the best so far) are set up once, **before** the loop. Only things that belong to a single item go inside.

### Keeping a running count and a record

A common pattern keeps two numbers: the current run, and the best run seen so far.

```python
best = 0
for score in [3, 9, 4]:
    if score > best:
        best = score
# best is 9
```

### Debugging technique: a trace table

Draw a small table with one column per variable and one row per loop pass. Fill it in by hand for a short input like `[True, True, False]`. When a number in the table stops matching what you expected, you've found the line to look at.
