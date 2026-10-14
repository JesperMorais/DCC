### Growing a buffer with `realloc`

A growable string keeps three things: a heap block (`data`), how many bytes are used (`len`), and how many it can hold (`cap`). Appending `n` bytes needs room for `len + n` characters plus the `'\0'`. When that doesn't fit, you `realloc` to a bigger capacity.

Growing by **doubling** keeps appends cheap on average: the block is copied rarely, and each copy is paid for by all the appends that filled it.

```c
char *bigger = realloc(buf, new_cap);   // may move the block
if (bigger == NULL) {
    return false;                       // buf is still valid and unchanged
}
buf = bigger;
```

Assigning `realloc`'s result to a temporary first matters: on failure it returns `NULL` but leaves the old block alive, and `buf = realloc(buf, ...)` would leak it.

### Debugging: test the sizes, not just the contents

Growth code has a few sizes that matter: empty, exactly full, one byte over, and **much** bigger than the current capacity. Small appends only ever test "one step over". Try each boundary and print `len` and `cap` after every append to check the invariant `len + 1 <= cap`.
