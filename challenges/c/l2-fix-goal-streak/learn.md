### Loops that carry state

Some loops need to remember something from one round to the next. This one counts how many separate **rainy spells** a month had (a spell is one or more rainy days in a row):

```c
#include <stdbool.h>
#include <stddef.h>

size_t rainy_spells(const int *rain_mm, size_t count) {
    size_t spells = 0;
    bool raining = false;
    for (size_t i = 0; i < count; i++) {
        if (rain_mm[i] > 0 && !raining) {
            spells++;            // a new spell starts today
        }
        raining = rain_mm[i] > 0;
    }
    return spells;
}
```

`raining` carries yesterday's weather into today's round. Finding the **longest** run works the same way, with two variables: the run you're in right now, and the best run so far. On each element the current run either grows or ends, and when it ends you compare it with the best one.

### The end of the array is a boundary too

Bugs love the edges of the data: the first element, the last element, an empty array. "What happens when something is still going when the loop stops?" is a question worth asking of every loop that keeps state.

### Debugging: a trace table

Pick a short input that reproduces the bug and make a table with one row per round of the loop and one column per variable:

| `i` | `value` | `current` | `longest` |
|---|---|---|---|
| 0 | 7 | 1 | 0 |
| 1 | 2 | 0 | 1 |
| … | | | |

Fill in the **end of each round**. Then add one last row: what does the function return? Compare it with the answer you expected and look for the first row where the numbers stop making sense.
