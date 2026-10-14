### Separate `if`s versus an `if / else if` chain

These two look alike, but behave differently:

```c
// Two separate ifs: BOTH are checked, every time.
if (score >= 50) { grade = 'C'; }
if (score >= 80) { grade = 'A'; }

// A chain: checked from the top, and only the FIRST true one runs.
if (score >= 80)      { grade = 'A'; }
else if (score >= 50) { grade = 'C'; }
```

With separate `if`s, a later block can **overwrite** what an earlier one did. Sometimes that's exactly what you want, and then the order of the `if`s decides who wins. With a chain, as soon as one condition is true, the rest are skipped.

A **variable** like `grade` keeps whatever was last stored in it. `grade = 'A';` throws away the old value.

### Debugging: follow the value

Pick the input from the bug report and walk through the code one line at a time. Keep a note of the variable:

```
score = 90
grade = ?          (start)
score >= 50 → yes, grade = 'C'
score >= 80 → yes, grade = 'A'
return 'A'
```

Each time the variable changes, ask: should it change here, for this input? The first change you didn't expect is the bug.
