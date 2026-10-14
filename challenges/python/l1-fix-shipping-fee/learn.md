### Choosing with `if`, `elif` and `else`

An `if` runs a block of code only when its condition is `True`. You can chain more choices with `elif` ("else if") and finish with `else` ("none of the above"):

```python
def describe(temperature: float) -> str:
    if temperature >= 30:
        return "hot"
    elif temperature >= 15:
        return "mild"
    else:
        return "cold"
```

Python checks the conditions **from top to bottom** and runs only the **first** one that is `True`. All the others are skipped, even if they would also be true. So for `35`, both `>= 30` and `>= 15` are true, but only `"hot"` is returned because that check comes first.

### Debugging technique: test the boundaries

Bugs in conditions hide at the edges. For every threshold in the rules (here 20 and 50), try a value just below it, exactly on it, and well above it. Then follow the code by hand and note which branch runs for each one.
