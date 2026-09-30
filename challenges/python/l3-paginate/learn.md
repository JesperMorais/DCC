### Slicing

A slice copies a stretch of a list: `items[start:stop]` takes the items from index `start` up to, but **not including**, `stop`.

```python
letters = ["a", "b", "c", "d"]
letters[1:3]    # ["b", "c"]
letters[:2]     # ["a", "b"]   start defaults to 0
letters[2:]     # ["c", "d"]   stop defaults to the end
letters[3:10]   # ["d"]        stop past the end is fine
letters[8:10]   # []           so is start past the end
```

Two things make slicing pleasant to work with:

- **It never raises `IndexError`.** Out-of-range bounds are clipped to the list, unlike `letters[8]`, which does raise.
- **It returns a new list**, so the original is untouched.

### Guard clauses

Check for bad input first and `raise` right away. The rest of the function can then assume the input is valid:

```python
if quantity < 0:
    raise ValueError("quantity must not be negative")
```
