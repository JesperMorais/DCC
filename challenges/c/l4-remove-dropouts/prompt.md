A weather station's temperature sensor sometimes drops out, and the firmware logs a **sentinel** value (like `-999`) instead of a reading. Before computing averages, those entries have to go. Write `remove_sentinel`:

```c
size_t remove_sentinel(int *readings, size_t count, int sentinel);
```

- Remove every element equal to `sentinel` **in place**, shifting the kept readings to the front **in their original order**.
- Return the **new length**. Elements at and after that index don't matter.
- If `readings` is `NULL`, return `0`.

```c
int temps[] = {21, -999, 22, -999, -999, 23};
size_t n = remove_sentinel(temps, 6, -999);
// n == 3, and temps now starts with {21, 22, 23}
```

Do it in a **single pass** without a second array.
