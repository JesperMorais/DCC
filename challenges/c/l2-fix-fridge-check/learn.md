### Loops that answer yes or no

Many functions loop over an array to answer a question: "is there a negative number?", "are all of these valid?". Those two questions behave differently:

- **"Is there any…?"** You can say **yes** the moment you find one. You can only say **no** after checking everything.
- **"Are all of them…?"** You can say **no** the moment one fails. You can only say **yes** after checking everything.

```c
#include <stdbool.h>
#include <stddef.h>

bool any_negative(const int *values, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (values[i] < 0) {
            return true;    // found one: done
        }
    }
    return false;           // looked at all of them, none matched
}
```

A `return` inside a loop ends the **whole function**, not just this round of the loop.

### Debugging: print inside the loop

To see how far a loop really gets, add a temporary `printf` as the first line of the loop body (and `#include <stdio.h>` at the top):

```c
printf("checking i=%zu value=%d\n", i, values[i]);
```

Run the tests and read the output. If you expected four lines and got one, the loop stopped early, and the question becomes *what made it stop?* Remove the `printf` once you've found the bug.
