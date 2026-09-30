### When `int` isn't enough

`int / int` is **whole-number** division in C. The fraction is simply thrown away:

```c
int passed = 7;
int total  = 8;
passed / total;            // 0     (!)
```

To keep decimals, at least one side of `/` has to be a `double`. You can convert a value with a **cast**, which is the type name in parentheses:

```c
(double)passed / total;            // 0.875
100.0 * passed / total;            // 87.5  (100.0 is already a double)
```

Note the order: `(double)(passed / total)` is still `0.0`, because the division already happened in `int` before the cast.

### Counts are `size_t`

Array lengths have the type `size_t`, an unsigned whole-number type. It converts to `double` the same way: `(double)count`.

### Guard before dividing

Dividing by zero is undefined for integers and gives nonsense for decimals, so check for an empty input **before** you divide. A guard at the top of the function keeps the rest simple:

```c
if (total == 0) {
    return 0.0;
}
```
