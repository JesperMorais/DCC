### A decorator is just a function that wraps a function

`@shout` above a `def` means `greet = shout(greet)`:

```python
def shout(fn: Callable[P, str]) -> Callable[P, str]:
    @functools.wraps(fn)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> str:
        return fn(*args, **kwargs).upper()
    return wrapper

@shout
def greet(name: str) -> str:
    return f"hi {name}"

greet("ada")   # "HI ADA"
```

### Decorators with arguments need one more layer

`@retry(times=3)` first *calls* `retry(times=3)`, and whatever that returns is used as the decorator. So there are three nested functions: the **factory** (takes the options), the **decorator** (takes `fn`), and the **wrapper** (takes the call's arguments). Each inner layer closes over the outer layer's variables.

### Why `functools.wraps`?

Without it, `greet.__name__` would be `"wrapper"` and the docstring would be gone. Logs, debuggers, `help()` and test reports would all show the wrong name. `@wraps(fn)` copies `__name__`, `__doc__`, `__module__`, … and sets `wrapper.__wrapped__ = fn`.

### Typing wrappers: `ParamSpec`

`[**P, R]` declares a *parameter specification*: "whatever parameters `fn` has". With `*args: P.args, **kwargs: P.kwargs` the wrapper has exactly the same signature as `fn`, so mypy still checks callers of the decorated function.

### `except` with a tuple

`except (KeyError, ValueError) as e:` catches any of those classes and their subclasses. A bare `raise` inside the `except` block re-raises the current exception unchanged.
