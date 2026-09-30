You're organising a party and keep the guest list in a Python list. Write `add_guest(guests, name, capacity)` that tries to add `name` to the list and returns what happened:

1. If `name` is **already on the list**, return `"already invited"` and don't add it again.
2. Otherwise, if the list already has `capacity` guests (or more), return `"full"`.
3. Otherwise, **append** `name` to `guests` and return `"added"`.

This function **changes the list it's given**. Check rule 1 before rule 2: someone already on a full list still gets `"already invited"`.

```python
guests = ["Ada", "Linus"]
add_guest(guests, "Grace", 3)   # "added"    and now guests == ["Ada", "Linus", "Grace"]
add_guest(guests, "Ada", 3)     # "already invited"
add_guest(guests, "Guido", 3)   # "full"
```
