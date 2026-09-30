A restaurant's freezer logs its temperature every hour. Food safety wants to know the **warmest** moment of the day. Write `highest_reading`:

```c
#include <stddef.h>

int highest_reading(const int *values, size_t count);
```

- Return the largest value among the first `count` readings.
- `count` is always at least `1`.
- Freezer readings are almost always **negative**. The warmest of `-18`, `-21` and `-15` is `-15`.

Examples:

```c
int freezer[] = {-18, -21, -15, -19};
highest_reading(freezer, 4);   // → -15

int fridge[] = {4, 7, 3};
highest_reading(fridge, 3);    // → 7
```
