### Two kinds of division

You already know `/`, which gives a `float`: `7 / 2` is `3.5`. Python has two more division operators for whole numbers:

- `//` is **integer division**. It tells you how many *whole* times one number fits into another.
- `%` is **modulo**. It gives you the **remainder**, whatever is left over.

Say you're packing 30 eggs into cartons of 12:

```python
eggs = 30
full_cartons = eggs // 12   # 2
loose_eggs = eggs % 12      # 6   (because 2 * 12 = 24, and 30 - 24 = 6)
```

Both give an `int`, so the result is a whole number.

### Padding with zeros

In an f-string, the format spec `:02d` means "show this whole number (`d`) with at least 2 characters, filling with zeros (`0`) in front":

```python
day = 7
f"Day {day:02d}"    # "Day 07"
f"{12:02d}"         # "12"  (already two digits, nothing added)
```
