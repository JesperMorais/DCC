A smart meter logs power usage in watts, and the billing system charges a surcharge on every reading above a limit. Write `sum_above`, which adds up the readings in a range that are **strictly greater** than `limit`:

```c
long sum_above(const int *begin, const int *end, int limit);
```

The range is **half-open**: `begin` points at the first reading and `end` points **one past the last one**. If `begin == end`, the range is empty. Walk it with a pointer, not an index.

```c
int watts[] = {120, 900, 450, 1300, 80};
sum_above(watts, watts + 5, 400);       // 900 + 450 + 1300 = 2650
sum_above(watts + 1, watts + 3, 500);   // just 900
sum_above(watts, watts, 0);             // 0: empty range
```
