### Repeating with `for` and `range`

A `for` loop runs the same code once for every value in a sequence. `range` makes a sequence of whole numbers:

```python
for number in range(1, 4):
    print(number)     # prints 1, then 2, then 3
```

`range(start, stop)` starts **at** `start` and stops **before** `stop`. The stop value itself is never included. So `range(1, 4)` is `1, 2, 3`, and `range(0, 0)` is empty.

### Adding things up

To add up values in a loop, start a variable at `0` **before** the loop and add to it inside:

```python
total = 0
for price in [3, 5, 2]:
    total += price    # same as: total = total + price
# total is now 10
```

### Debugging technique: print what the loop sees

If a loop gives the wrong answer, put a `print(...)` inside it and run it with a **small** input. Seeing every value the loop visits often shows straight away that one is missing or extra. Remember to remove the print afterwards.
