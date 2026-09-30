### Repeating work with `for`

A `for` loop runs the same block many times. Its header has three parts, separated by `;`:

```c
for (int i = 0; i < 3; i++) {
    // runs with i = 0, then 1, then 2
}
```

1. **start**: `int i = 0` creates a counter that only exists inside the loop
2. **keep going while**: `i < 3` is checked *before* every round, and the loop stops as soon as it's false
3. **step**: `i++` (add 1 to `i`) runs *after* every round

If the condition is false from the start, the body never runs at all.

### Collecting a result

To build one answer from many rounds, create a variable **before** the loop and update it **inside**:

```c
int multiples_of_7 = 0;
for (int i = 1; i <= 50; i++) {
    if (i % 7 == 0) {
        multiples_of_7++;
    }
}
// multiples_of_7 is 7  (7, 14, 21, 28, 35, 42, 49)
```

Shortcuts you'll see everywhere: `x++` means `x = x + 1`, and `x += 5` means `x = x + 5`.

Pick your bounds carefully: `<` stops *before* the end value, and `<=` includes it.
