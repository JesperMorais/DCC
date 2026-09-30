A sales report arrives as one row per store and one column per weekday, but the dashboard wants one row per weekday and one column per store. Write `transpose`, which flips a table so its rows become columns:

```c
void transpose(size_t rows, size_t cols,
               const int in[rows][cols], int out[cols][rows]);
```

- `in` is a `rows × cols` table, and the caller provides `out`, a `cols × rows` table, to fill in.
- After the call, `out[c][r] == in[r][c]` for every `r` and `c`.

```c
const int sales[2][3] = {
    {10, 20, 30},   // store A: Mon, Tue, Wed
    {40, 50, 60},   // store B
};
int by_day[3][2];
transpose(2, 3, sales, by_day);
// by_day == {{10, 40}, {20, 50}, {30, 60}}
```

The tables are usually **not square**, so keep your indices straight.
