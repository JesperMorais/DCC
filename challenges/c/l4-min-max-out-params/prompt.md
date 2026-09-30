Write `min_max`, which finds the smallest **and** the largest value in an array in one pass:

```c
bool min_max(const int *values, size_t count, int *min, int *max);
```

- On success, store the results through the `min` and `max` pointers and return `true`.
- If the array is empty (`count == 0`) or `values` is `NULL`, return `false` and **leave `*min` and `*max` unchanged**.

```c
int temps[] = {3, -2, 9, 4};
int lo, hi;
if (min_max(temps, 4, &lo, &hi)) {
    /* lo == -2, hi == 9 */
}
```

`const int *values` promises you won't modify the caller's array. The compiler holds you to that.
