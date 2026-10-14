### Functions that remember: closures

A function defined inside another function can use the outer function's variables. Return it, and it still has access to them. That's a **closure**:

```python
def make_greeter(greeting: str) -> Callable[[str], str]:
    def greet(name: str) -> str:
        return f"{greeting}, {name}!"
    return greet

hello = make_greeter("Hello")
hello("Ada")   # "Hello, Ada!"
```

The important detail: a closure remembers the **variable**, not the value it had when the inner function was created. Python looks the name up each time the inner function **runs**. Usually that makes no difference. It matters when the outer variable keeps changing after the inner function was made.

Default parameter values are different: they're evaluated **once**, when `def` runs, and stored with the function.

### Debugging technique: test with more than one

Code that builds several things in a loop often passes every test that builds just one. Make a minimal case with two or three different inputs and compare each output:

```python
fns = make_tax_calculators({"A": 0.1, "B": 0.5})
print(fns["A"](100), fns["B"](100))
```

If they all behave like the last one, ask *when* each function reads its data.
