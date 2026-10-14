### The "best so far" pattern

To find the largest, smallest, longest… of an array, you keep one variable holding the **best value seen so far**, and update it whenever you meet a better one:

```c
#include <stddef.h>

int largest_order(const int *orders, size_t count) {
    int largest = orders[0];
    for (size_t i = 1; i < count; i++) {
        if (orders[i] > largest) {
            largest = orders[i];
        }
    }
    return largest;
}
```

The loop is only half the story. The **starting value** matters just as much: the answer can only ever be the starting value or something that beat it. So the starting value must be something that is allowed to be the answer, or something every real value is guaranteed to beat.

### Debugging: vary the data, not just the size

When code works on some inputs and fails on others, line the two up and ask what's different about them. Are all the numbers positive? All negative? Is the answer the first element, or the last? Then write the smallest input that has the "bad" property, like an array with a single reading, and trace it by hand.
