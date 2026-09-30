### Repeating until something changes: `while`

A `for` loop runs once per item. A `while` loop keeps going **as long as a condition is `True`**, which is handy when you don't know up front how many steps you need:

```python
savings = 100.0
years = 0
while savings < 200:
    savings = savings * 1.1   # 10% interest per year
    years += 1

years   # 8
```

Python checks the condition before every round. Something inside the loop **must** eventually make it `False`, otherwise the loop never ends. (daily.ts stops any test that runs longer than 1.5 s.)

### Handy tools for whole numbers

```python
365 // 100   # 3    (how many whole hundreds)
365 % 100    # 65   (what's left over)
x = 50
x //= 2      # same as x = x // 2, so x is now 25
abs(-8)      # 8    (the distance from zero, always positive)
```

Try `% 10` and `// 10` on a few numbers of your own and see what they give you.
