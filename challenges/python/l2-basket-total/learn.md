### Dictionaries

A **dict** stores pairs of **keys** and **values**, so you can look something up by name instead of by position:

```python
dial_codes = {"Sweden": 46, "Norway": 47, "Japan": 81}
dial_codes["Norway"]   # 47
```

The type hint for this dict is `dict[str, int]`: the keys are strings and the values are ints.

### Missing keys

Looking up a key that isn't there with `[...]` crashes:

```python
dial_codes["Mars"]   # KeyError: 'Mars'
```

`.get(key, default)` is the safe version. It returns the value if the key exists, and `default` if it doesn't:

```python
dial_codes.get("Japan", 0)   # 81
dial_codes.get("Mars", 0)    # 0
dial_codes.get("Mars")       # None (the default default)
```

You can also ask first with `in`: `"Mars" in dial_codes` is `False`. `in` checks the **keys** of a dict, not its values.
