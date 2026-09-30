A supermarket stacks tins of soup into a pyramid: the top row has **1** can, the next row has **2**, and so on down to the bottom row, which has **`n`** cans. How many cans does a pyramid with `n` rows need?

Write `sum_one_to_n`, which returns 1 + 2 + … + n:

```c
int sum_one_to_n(int n);
```

- If `n` is `0` or negative, there's no pyramid, so return `0`.

Examples:

- `sum_one_to_n(4)` → `10` (1 + 2 + 3 + 4)
- `sum_one_to_n(1)` → `1`
- `sum_one_to_n(0)` → `0`

Use a loop. (There's a famous shortcut formula, but today is about loops.)
