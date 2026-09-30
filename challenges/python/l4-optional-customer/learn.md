### "Maybe a value": `X | None`

A lookup that can come up empty should say so in its signature:

```python
def find_room(rooms: list[str], code: str) -> str | None:
    for room in rooms:
        if room.startswith(code):
            return room
    return None
```

`str | None` (the older spelling is `Optional[str]`) tells every caller, and mypy, that they **must** handle the `None` case.

### Narrowing

mypy follows your `if` checks. Inside `if x is None:` it knows `x` is `None`. After that branch returns, it knows `x` can only be the other type:

```python
room = find_room(rooms, "B2")
room.upper()            # mypy error: "None" has no attribute "upper"

if room is None:
    return "no such room"
return room.upper()     # fine, room is a str here
```

Prefer `is None` over `if not room:`. An empty string is falsy too, and that's a different situation.

### `or` for fallbacks

`a or b` gives `a` if it's truthy, else `b`, so `None` and `""` both fall through:

```python
label = custom_label or default_label
```
