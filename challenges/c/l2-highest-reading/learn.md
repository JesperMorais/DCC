### Remembering "the best so far"

A lot of array problems boil down to walking through the elements while one variable remembers the best candidate found **so far**. Here we look for the **earliest** hour at which a shop got busy (more than 20 customers):

```c
#include <stddef.h>

int first_busy_hour(const int *customers, size_t count) {
    int answer = -1;                 // -1 means "never busy"
    for (size_t i = 0; i < count; i++) {
        if (customers[i] > 20 && answer == -1) {
            answer = (int)i;
        }
    }
    return answer;
}
```

The variable lives **outside** the loop, so it keeps its value from one round to the next.

### The starting value matters

Whatever you put in the variable before the loop has to be a value that can't win by accident. For a *sum*, `0` is perfect. For a "biggest so far", ask yourself: is there any input where your starting value is bigger than **every** real element? If so, your function returns a number that isn't even in the array.

One safe choice is always available: an element that is actually in the array.

### Starting a loop later

A `for` loop doesn't have to start at `0`. `for (size_t i = 1; i < count; i++)` skips the first element, which is useful when you've already dealt with it.
