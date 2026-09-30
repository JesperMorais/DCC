### Closures remember their birthplace

A function defined inside another function can see the outer function's local variables, and it keeps them alive after the outer function returns. Each call to the outer function creates a **fresh** set of variables:

```python
def make_greeter(greeting: str) -> Callable[[str], str]:
    def greet(name: str) -> str:
        return f"{greeting}, {name}!"      # reads the enclosing variable
    return greet

hi, yo = make_greeter("Hi"), make_greeter("Yo")
hi("Ada")   # "Hi, Ada!"
```

### Reading is free, rebinding needs `nonlocal`

Assigning to a name inside a function makes it **local to that function** by default. So this breaks:

```python
def make_counter() -> Callable[[], int]:
    count = 0
    def tick() -> int:
        count += 1        # UnboundLocalError: count is now a *local* of tick
        return count
    return tick
```

`nonlocal count` tells Python "I mean the variable from the enclosing function":

```python
    def tick() -> int:
        nonlocal count
        count += 1
        return count
```

Mutating an object (`items.append(x)`) never needed `nonlocal`. Only **rebinding the name** (`=`, `+=`) does.

### Inject the clock

Code that calls `time.monotonic()` directly is hard to test. Taking a `clock` callable (defaulting to the real one) lets a test move time forward instantly and deterministically.
