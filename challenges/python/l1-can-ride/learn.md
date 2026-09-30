### True or False

A **boolean** (`bool`) is a value that is either `True` or `False`. You get one whenever you compare two things:

| Operator | Means | Example | Result |
|---|---|---|---|
| `==` | equal to | `3 == 3` | `True` |
| `!=` | not equal to | `3 != 3` | `False` |
| `<` / `>` | less / greater than | `2 > 5` | `False` |
| `<=` / `>=` | less / greater than or equal | `5 >= 5` | `True` |

Note that `=` stores a value, while `==` *asks* whether two values are equal.

### A comparison is a value

You can store a comparison in a variable or `return` it directly, just like a number:

```python
def is_freezing(temp_c: float) -> bool:
    return temp_c <= 0

is_freezing(-3.0)  # True
is_freezing(12.0)  # False
```

### Combining checks

- `a and b` is `True` only when **both** are `True`.
- `a or b` is `True` when **at least one** is `True`.
- `not a` flips `True` and `False`.

```python
day = "Sat"
is_weekend = day == "Sat" or day == "Sun"   # True
```
