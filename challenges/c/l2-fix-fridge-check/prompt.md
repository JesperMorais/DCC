**Bug report from the café's food-safety officer:** "The fridge log for Tuesday says everything was fine, but I can see a reading of 9 °C in the middle of the afternoon. Anything above 8 should have flagged the day."

`all_readings_in_range` checks a day of temperature readings against the allowed range:

```c
#include <stdbool.h>
#include <stddef.h>

bool all_readings_in_range(const int *readings, size_t count, int low, int high);
```

- Return `true` if **every** reading is between `low` and `high`, **inclusive** (exactly `low` or `high` is fine).
- Return `false` as soon as any single reading is outside the range.
- A day with no readings (`count == 0`) has nothing wrong in it: return `true`.

What it should do:

```c
int monday[]  = {3, 4, 5, 4};
int friday[]  = {-1, 4, 5};
all_readings_in_range(monday, 4, 0, 8);   // → true
all_readings_in_range(friday, 3, 0, 8);   // → false (-1 is too cold)
all_readings_in_range(monday, 0, 0, 8);   // → true  (no readings)
```

**Reproduction:**

```c
int tuesday[] = {4, 5, 9, 6};
all_readings_in_range(tuesday, 4, 0, 8);   // returns true, expected false
```

Find the bug and fix it.
