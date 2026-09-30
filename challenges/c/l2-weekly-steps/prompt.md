A fitness tracker stores how many steps you walked each day in an array. Write `total_steps`, which adds up the first `count` entries:

```c
#include <stddef.h>

int total_steps(const int *steps, size_t count);
```

- If `count` is `0` (no days recorded yet), return `0`.
- Only look at the first `count` entries.

Examples:

```c
int week[] = {8000, 10250, 4300, 12000, 9100, 15600, 3000};
total_steps(week, 7);   // → 62250
total_steps(week, 2);   // → 18250 (just Monday and Tuesday)
total_steps(week, 0);   // → 0
```
