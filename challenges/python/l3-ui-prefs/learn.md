### `dict.get` with a default

`d[key]` raises `KeyError` when the key is missing. `d.get(key, default)` returns the default instead:

```python
stock = {"apples": 3}
stock.get("apples", 0)   # 3
stock.get("pears", 0)    # 0
stock.get("pears")       # None  (the default default)
```

### Walking nested data safely

For data several levels deep, go down **one level at a time** and keep each step safe:

```python
order = {"shipping": {"address": {"city": "Oslo"}}}
shipping = order.get("shipping", {})
address = shipping.get("address", {})
city = address.get("city", "unknown")
```

### Missing vs. `None`

`.get(key, {})` only helps when the key is **absent**. If the key is there with the value `None`, you get `None` back, and calling `.get` on that crashes. `x or {}` swaps any falsy value (`None`, `{}`) for an empty dict:

```python
{"shipping": None}.get("shipping", {})    # None
{"shipping": None}.get("shipping") or {}  # {}
```

### Typing JSON-ish data

JSON can hold anything, so `dict[str, Any]` (from `typing`) is the honest type for it.
