### An `if` inside a loop

A loop visits every element. Put an `if` inside it, and you only act on the elements you care about. Here we add up **only the positive** transactions in a bank statement, which gives the total money that came in:

```c
#include <stddef.h>

int money_in(const int *amounts, size_t count) {
    int total = 0;
    for (size_t i = 0; i < count; i++) {
        if (amounts[i] > 0) {
            total += amounts[i];
        }
    }
    return total;
}
```

The loop runs `count` times, but `total` only changes on the rounds where the condition is true.

### Counting instead of summing

If you want to know **how many** elements match rather than what they add up to, add `1` instead of the element itself. `n++` is the usual way to write "add one to `n`".

A count can never be negative and it's about the elements of an array, so `size_t` is the natural type for it, the same type as `count`.

### `>` versus `>=`

"More than 50" and "50 or more" differ by exactly one value. Read the rule carefully, and test the boundary value itself.
