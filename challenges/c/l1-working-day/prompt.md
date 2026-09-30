A timesheet app numbers the days of the week from **1 (Monday)** to **7 (Sunday)**. Write `is_working_day`, which says whether people are expected at work:

```c
#include <stdbool.h>

bool is_working_day(int day);
```

- Monday to Friday (`1`–`5`) → `true`
- Saturday and Sunday (`6`, `7`) → `false`
- Any other number isn't a real day (`0`, `8`, `-1`…) → `false`

Examples:

- `is_working_day(3)` → `true` (Wednesday)
- `is_working_day(7)` → `false` (Sunday)
- `is_working_day(9)` → `false` (not a day at all)

This is a perfect fit for a `switch` statement.
