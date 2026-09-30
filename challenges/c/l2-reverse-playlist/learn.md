### Functions that fill an array

A C function can only `return` one value, and it can't return a whole array. Instead, the **caller** creates the result array and passes it in, and the function writes into it:

```c
#include <stddef.h>

void double_all(const int *src, size_t count, int *out) {
    for (size_t i = 0; i < count; i++) {
        out[i] = src[i] * 2;
    }
}

int prices[] = {5, 12, 30};
int doubled[3];                  // room for 3 ints, contents not set yet
double_all(prices, 3, doubled);  // doubled is now {10, 24, 60}
```

- `void` means the function returns nothing. Its result lives in `out`.
- `const int *src`: read-only input. `int *out` (no `const`): the function may write to it.
- The caller is responsible for making `out` big enough. The function has to stay inside `0 … count - 1`.

### Reading and writing at different positions

The index you write to doesn't have to be the index you read from. `out[i] = src[i + 1]` would shift everything one step left (and read one past the end on the last round, so beware!). Try a tiny example on paper: for each `i`, which `src` index do you need?

### `size_t` never goes negative

`size_t` is unsigned. `0 - 1` doesn't give `-1`, it **wraps around** to an enormous number. So be careful with expressions like `count - 1` when `count` might be `0`.
