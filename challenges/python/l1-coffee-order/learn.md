### Default parameters

A **parameter** is an input a function expects. You can give a parameter a **default value** in the `def` line. If the caller leaves it out, the default is used:

```python
def brew_tea(tea: str, minutes: int = 3) -> str:
    return f"Steep the {tea} for {minutes} min"

brew_tea("green")        # "Steep the green for 3 min"
brew_tea("black", 5)     # "Steep the black for 5 min"
```

Parameters with defaults must come **after** the ones without.

### Keyword arguments

When calling, you can name an argument. This lets you skip earlier defaults and set just the one you care about:

```python
def alarm(hour: int = 7, minute: int = 0, snooze: bool = True) -> str:
    ...

alarm(snooze=False)   # hour=7, minute=0, snooze=False
alarm(6, minute=30)   # hour=6, minute=30, snooze=True
```

### Choosing between two values

For a quick either/or, Python has a one-line `if`, written as *value-if-true* `if` *condition* `else` *value-if-false*:

```python
label = "even" if n % 2 == 0 else "odd"
```
