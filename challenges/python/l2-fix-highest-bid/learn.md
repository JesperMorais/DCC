### Text that looks like a number

Data from forms, files and the command line arrives as **strings**, even when it looks like a number. `"120"` and `120` are different things in Python:

```python
120 + 1        # 121
"120" + "1"    # "1201"
int("120") + 1 # 121
```

### How strings compare

Strings compare **character by character**, like words in a dictionary. The first character that differs decides, and the length only matters if one string runs out first:

```python
"apple" < "banana"   # True:  "a" comes before "b"
"9" < "10"           # False: "9" comes after "1"
9 < 10               # True
```

That's why sorting or taking the `max` of numeric strings gives surprising results. Convert them with `int(...)` **before** comparing.

### Debugging technique: check the types

When a comparison gives a strange result, print the values together with their types: `print(repr(x), type(x))`. `repr` shows strings with quotes, so `'9'` and `9` look different.
