**Bug report from a weather-app user:** "In July the app said the coldest night of the week was 0 °C. It was 14 °C at the lowest! In January the numbers look right."

`coldest_reading` returns the lowest temperature in an array of readings:

```c
#include <stddef.h>

int coldest_reading(const int *readings, size_t count);
```

There is always **at least one** reading (`count >= 1`). Readings can be negative.

What it should do:

```c
int january[] = {-3, -11, -7, -2};
int march[]   = {4, -1, 6};
coldest_reading(january, 4);   // → -11
coldest_reading(march, 3);     // → -1
```

**Reproduction:**

```c
int july[] = {17, 14, 19, 16};
coldest_reading(july, 4);      // returns 0, expected 14
```

Find the bug and fix it.
