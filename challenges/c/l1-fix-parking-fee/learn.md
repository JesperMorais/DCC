### Reading someone else's function

A **function** takes inputs (its **parameters**), works something out, and hands back one result with `return`. A **variable** is a named box holding a value:

```c
int cinema_snacks(int people) {
    int popcorn_price = 40;
    int free_tubs = 1;
    if (people <= free_tubs) {
        return 0;
    }
    return (people - free_tubs) * popcorn_price;
}
```

The computer runs it top to bottom. An `if` decides whether a block runs, and a `return` ends the function right there, so nothing after it runs.

### Debugging: be the computer

When a function gives a wrong answer, don't stare at it. **Run it by hand** with the input from the bug report, and write down every variable's value as you go:

| line | `people` | `popcorn_price` | `free_tubs` | result |
|---|---|---|---|---|
| start | 3 | | | |
| after the variables | 3 | 40 | 1 | |
| `if` is false, so skip it | | | | |
| last `return` | | | | (3 − 1) × 40 = 80 |

Then check: is that what a person would have calculated? The first line where your table and your own common sense disagree is where the bug is.
