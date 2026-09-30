### `while`: loop until something changes

A `for` loop is great when you know how many rounds you need. When you **don't**, and you just want to keep going until a condition changes, use `while`:

```c
int halvings = 0;
int size = 100;
while (size > 1) {
    size = size / 2;   // 100 → 50 → 25 → 12 → 6 → 3 → 1
    halvings++;
}
// halvings is 6
```

The condition is checked **before** each round. If it's false right away, the body runs **zero** times, so always ask yourself what happens with the smallest possible input.

### Integer division peels off digits

With `int`s, `/ 10` drops the last decimal digit and `% 10` gives you that digit:

```c
int year = 1984;
year % 10;   // 4
year / 10;   // 198
```

In C, integer division always **truncates towards zero**: `-37 / 10` is `-3`, not `-4`. So a negative number shrinks towards 0 the same way a positive one does.

### Shorthand

`n /= 10` means `n = n / 10`. The same works for `+=`, `-=`, `*=` and `%=`.
