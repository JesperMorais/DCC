### Functions

A function is a named, reusable piece of code. You give it inputs (**parameters**) and it gives back an output with `return`:

```python
def double(n: int) -> int:
    return n * 2

double(4)  # 8
```

Python uses **indentation** (4 spaces) to know which lines belong to the function. There are no curly braces.

### Type hints

`n: int` and `-> int` are *type hints*. They document what goes in and what comes out. Python doesn't enforce them while running, but tools like **mypy** (which daily.ts runs for you) check them and warn you about mistakes.

### f-strings

```python
city = "Oslo"
msg = f"Welcome to {city}!"  # "Welcome to Oslo!"
```
