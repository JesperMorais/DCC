### 2D arrays are rows laid end to end

`int grid[2][3]` is an array of 2 rows, each an array of 3 `int`s. In memory it's just 6 ints in a row: row 0 first, then row 1. `grid[r][c]` picks row `r`, then column `c`.

```c
int grid[2][3] = {
    {1, 2, 3},
    {4, 5, 6},
};
grid[1][0];   // 4
```

### Passing them to functions

To index `grid[r][c]`, the compiler has to know how long a row is, so the column count is part of the parameter type. Since C99 you can let the sizes come from earlier parameters (a *variable-length array* parameter):

```c
long column_total(size_t rows, size_t cols, const int t[rows][cols], size_t c) {
    long sum = 0;
    for (size_t r = 0; r < rows; r++) {
        sum += t[r][c];
    }
    return sum;
}
```

The size parameters must come **before** the array that uses them.

### Stay inside the lines

Nothing checks your indices. With a `2 × 3` table, `t[0][3]` quietly reads `t[1][0]`, and `t[2][0]` is past the end entirely. The loop bounds have to match each dimension exactly.

(One C17 wrinkle: a plain `int[2][3]` can't be passed where `const int[][3]` is expected without a warning, so callers declare the input `const` too.)
