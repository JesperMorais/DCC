A battery-management chip keeps running totals of charge in `int`s. A wrapped-around total could make it report "full" on an empty battery, so every update has to be **checked**:

```c
bool checked_add(int a, int b, int *out);
bool checked_sub(int a, int b, int *out);
```

- If the exact mathematical result of `a + b` (or `a - b`) fits in an `int`, store it in `*out` and return `true`.
- Otherwise return `false` and **leave `*out` unchanged**.
- Your code must never overflow itself. Signed overflow is undefined behavior, and UBSan stops the test the moment it happens.
- Use only `int` arithmetic and the limits from `<limits.h>`. No `long long`, no `__builtin_add_overflow`.

```c
int r = 0;
checked_add(2000000000, 100000000, &r);   // → true, r = 2100000000
checked_add(INT_MAX, 1, &r);              // → false, r unchanged
checked_sub(-1, INT_MIN, &r);             // → true, r = INT_MAX
checked_sub(0, INT_MIN, &r);              // → false
```
