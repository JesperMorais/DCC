A greenhouse sensor logs whole-degree temperature readings. Write `average_reading`, which returns their average **as a decimal number**:

```c
#include <stddef.h>

double average_reading(const int *readings, size_t count);
```

- The average is the sum of the readings divided by `count`. Keep the fraction: the average of `20` and `21` is `20.5`, not `20`.
- If `count` is `0`, return `0.0`.
- Readings can be negative: the greenhouse is unheated in winter.

Examples:

```c
int day[] = {20, 21, 22, 23};
average_reading(day, 4);    // → 21.5

int night[] = {-4, -5};
average_reading(night, 2);  // → -4.5
```
