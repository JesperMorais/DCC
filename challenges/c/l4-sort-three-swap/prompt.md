A podium display gets three race times in whatever order the timing gates reported them. Write two functions:

```c
void swap_ints(int *a, int *b);
void sort_three(int *a, int *b, int *c);
```

- `swap_ints` exchanges the two integers that `a` and `b` point to.
- `sort_three` rearranges the three integers so that `*a <= *b <= *c`. Use `swap_ints` to do it.

```c
int x = 3, y = 7;
swap_ints(&x, &y);          // x == 7, y == 3

int gold = 58, silver = 51, bronze = 64;
sort_three(&gold, &silver, &bronze);   // gold == 51, silver == 58, bronze == 64
```

`swap_ints(&x, &x)` must leave `x` unchanged.
