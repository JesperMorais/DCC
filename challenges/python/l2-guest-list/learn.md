### Is it in there? `in`

`in` checks whether a value appears in a list, and gives you a `bool`:

```python
todos = ["buy milk", "call mum"]
"call mum" in todos       # True
"walk dog" in todos       # False
"walk dog" not in todos   # True
```

It works on strings too: `"ell" in "hello"` is `True`.

### How many? `len`

`len(...)` gives the number of items in a list (or characters in a string):

```python
len(todos)   # 2
len([])      # 0
```

### Changing a list

`.append(x)` adds `x` to the end of the list **in place**. It changes the list itself and returns `None`:

```python
todos.append("walk dog")
todos   # ["buy milk", "call mum", "walk dog"]
```

Lists are shared, not copied. If a function appends to a list it was given, the caller sees the change too.
