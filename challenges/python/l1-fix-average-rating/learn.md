### Two ways to divide

Python has **two** division operators, and they give different answers:

```python
7 / 2    # 3.5   normal division, always gives a float
7 // 2   # 3     floor division, rounds DOWN to a whole number
6 / 2    # 3.0   still a float, even when it divides evenly
```

`//` is handy when you really want whole numbers, like "how many full boxes of 6 can I fill with 20 eggs?" (`20 // 6` is `3`). When you want the exact answer, use `/`.

### Debugging technique: try the examples by hand

When a function gives the wrong answer, take one failing example and **work through the code line by line** with those exact numbers, writing down every value. Ask "is that what I expected?" after each line. Most bugs show up the moment a value looks wrong.

Pick an example where the answer is *not* a whole number. Inputs like `(10, 2)` hide some bugs because the answer happens to come out even.
