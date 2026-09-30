A calendar app needs to know whether February has 29 days. Write `is_leap_year`:

```c
#include <stdbool.h>

bool is_leap_year(int year);
```

The rules of the Gregorian calendar:

1. A year divisible by **4** is a leap year…
2. …**except** years divisible by **100**, which are not…
3. …**unless** they're also divisible by **400**, which are leap years after all.

Examples:

- `is_leap_year(2024)` → `true` (divisible by 4)
- `is_leap_year(2023)` → `false`
- `is_leap_year(1900)` → `false` (divisible by 100 but not 400)
- `is_leap_year(2000)` → `true` (divisible by 400)
