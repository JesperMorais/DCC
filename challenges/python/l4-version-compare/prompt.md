A package manager has to pick the newest release, but comparing version **strings** gets it wrong: `"1.10.0" < "1.9.2"` is `True` as text. Write a `Version` class that compares like a human would.

- `Version(major, minor, patch)` stores three ints as the attributes `major`, `minor` and `patch`.
- `Version.from_string(text)` is a **classmethod** that parses `"1.10.0"`. An optional leading `"v"` is allowed (`"v2.0.1"`). Anything else that isn't exactly three dot-separated non-negative integers raises `ValueError` with a message containing `"invalid version"`.
- Versions support `==`, `<`, `>`, `<=`, `>=` and `sorted()`, comparing major, then minor, then patch **numerically**.
- A `Version` is never equal to a non-`Version` (`Version(1, 0, 0) == "1.0.0"` is `False`).
- `repr(Version(1, 10, 0))` is `"Version('1.10.0')"`.

```python
Version.from_string("1.10.0") > Version.from_string("1.9.2")  # True
Version.from_string("v2.0.0") == Version(2, 0, 0)             # True
sorted([Version(1, 2, 0), Version(0, 9, 9)])  # [Version('0.9.9'), Version('1.2.0')]
Version.from_string("1.2")    # ValueError: invalid version: '1.2'
```
