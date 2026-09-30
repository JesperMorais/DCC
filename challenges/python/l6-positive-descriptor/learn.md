### Descriptors: attributes with behaviour

When a **class attribute** is an object that defines `__get__` / `__set__`, Python routes attribute access on instances through it:

| You write | Python calls |
|---|---|
| `obj.attr` | `type(obj).attr.__get__(obj, type(obj))` |
| `obj.attr = v` | `type(obj).attr.__set__(obj, v)` |
| `Cls.attr` | `Cls.attr.__get__(None, Cls)` (so `obj is None` means "accessed on the class") |

`@property`, methods, `classmethod` and `dataclasses.field` are all built on this.

### One descriptor, many instances

The descriptor object is created **once**, in the class body, and shared by *every* instance. So a value stored on the descriptor itself (`self.value = v`) is shared too, which is a classic bug. Per-instance data belongs on `obj`, for example in `obj.__dict__`. Because a descriptor with `__set__` is a *data descriptor*, it takes precedence over the instance's `__dict__`, so storing under the same name doesn't cause a loop.

### `__set_name__`: learning your own name

```python
class Logged:
    def __set_name__(self, owner: type, name: str) -> None:
        print(f"I am {owner.__name__}.{name}")

class Order:
    total = Logged()      # prints "I am Order.total"
```

Python calls `__set_name__` once when the owning class is created, so the descriptor finds out which attribute it's bound to. Use it for storage keys and error messages.

### Typing `__get__`

`@overload`s tell mypy that `Cls.attr` is the descriptor while `obj.attr` is a `float`. They're already in the starter.
