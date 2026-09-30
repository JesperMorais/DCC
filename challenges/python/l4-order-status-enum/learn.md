### Enums: a closed set of named values

An `Enum` lists every allowed value once. After that, a typo is an `AttributeError` or a mypy error instead of a silent bug:

```python
from enum import Enum

class Size(Enum):
    SMALL = "s"
    MEDIUM = "m"
    LARGE = "l"

Size.SMALL          # <Size.SMALL: 's'>
Size.SMALL.name     # "SMALL"
Size.SMALL.value    # "s"
Size("m")           # Size.MEDIUM   lookup by value
Size("xl")          # ValueError: 'xl' is not a valid Size
list(Size)          # [Size.SMALL, Size.MEDIUM, Size.LARGE]  definition order
```

Members are singletons, so compare them with `is` or `==`.

### Enums can have methods

An Enum is still a class. Methods receive the member as `self`:

```python
class Size(Enum):
    ...
    def is_big(self) -> bool:
        return self is Size.LARGE
```

To return a member of the class from inside it, annotate the return type as `"Size"` (a string, because the class isn't fully defined yet) or use `typing.Self`.
