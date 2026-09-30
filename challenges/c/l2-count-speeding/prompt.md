A speed camera records the speed (in km/h) of every car that passes. Write `count_speeding`, which returns how many cars went **faster than** the limit:

```c
#include <stddef.h>

size_t count_speeding(const int *speeds, size_t count, int limit);
```

- A car counts only if its speed is **strictly greater** than `limit`. Driving exactly at the limit is fine.
- Only look at the first `count` speeds.
- If `count` is `0`, nobody drove past, so return `0`.

Examples:

```c
int cars[] = {48, 52, 50, 67, 31};
count_speeding(cars, 5, 50);   // → 2 (52 and 67)
count_speeding(cars, 5, 70);   // → 0
count_speeding(cars, 5, 30);   // → 5
```
