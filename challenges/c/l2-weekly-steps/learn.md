### Arrays

An **array** is a row of values of the same type, stored side by side in memory:

```c
int temps[] = {12, 18, 9};
temps[0];   // 12: the first element is at index 0
temps[2];   // 9:  the last element is at index 2 (length - 1)
```

### Passing an array to a function

When you pass an array to a function, C hands over only **where it starts**, not how long it is. That's why C functions take the length as a separate parameter:

```c
#include <stdbool.h>
#include <stddef.h>

bool any_below_zero(const int *temps, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (temps[i] < 0) {
            return true;
        }
    }
    return false;
}
```

- `const int *temps` means "an array of `int`s that this function promises **not to change**". You still read it with `temps[i]`.
- `size_t` (from `<stddef.h>`) is C's type for sizes and counts. It's never negative, and it's the natural type for an array index.
- `i < count` visits indices `0` to `count - 1`, which is exactly the valid ones.

The function trusts `count`. Reading past the end is a real bug in C, and the test harness here will catch it.
