A report prints numbers in neat, right-aligned columns, so it needs to know how wide each number is. Write `count_digits`, which returns how many digits an `int` has:

```c
int count_digits(int n);
```

- `0` has one digit.
- For negative numbers, count only the digits. The minus sign isn't a digit.

Examples:

- `count_digits(2026)` → `4`
- `count_digits(7)` → `1`
- `count_digits(0)` → `1`
- `count_digits(-4096)` → `4`
