### Collecting extra arguments

A `*` parameter gathers any extra **positional** arguments into a tuple. A `**` parameter gathers any extra **keyword** arguments into a dict:

```python
def order(dish: str, *extras: str, **notes: object) -> None:
    print(dish, extras, notes)

order("pizza", "olives", "basil", table=4, rush=True)
# pizza ('olives', 'basil') {'table': 4, 'rush': True}
```

The annotation describes **each item**: `*extras: str` means every extra positional argument is a `str`, and `**notes: object` means keyword values can be anything. Keyword arguments keep the order the caller wrote them in.

### Unpacking when you call

In a **call**, the same symbols do the reverse. They spread a sequence or dict back out into separate arguments:

```python
dishes = ("olives", "basil")
opts = {"table": 4}
order("pizza", *dishes, **opts)   # same as order("pizza", "olives", "basil", table=4)
```

That lets a wrapper function forward "whatever it was given" to another function without knowing the details.
