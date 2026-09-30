A weather station counts how often the temperature **went up** during the day: how many readings are strictly higher than the reading just before them.

```c
#include <stddef.h>

size_t count_rises(const int *temps, size_t count);
```

```c
int today[] = {12, 14, 13, 15, 15, 18};
count_rises(today, 6);   // → 3 (12→14, 13→15 and 15→18; 15→15 is not a rise)
```

- A day with fewer than two readings has no rises, so return `0`.
- Only the first `count` readings belong to today.

**A colleague already wrote this function**, and it's in the editor. "It works on my machine," they say, and sometimes it even prints the right number. Run the tests: AddressSanitizer disagrees. Find the bug and fix it. Change as little as you can.
