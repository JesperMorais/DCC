### Walking a string with a flag

Many string tasks are about **transitions**: where a word starts, where a number ends, where a quote opens. You can't see a transition by looking at one character alone, so you carry a `bool` that remembers what the previous character was like:

```c
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

size_t count_digit_groups(const char *s) {
    size_t groups = 0;
    bool in_number = false;
    for (size_t i = 0; s[i] != '\0'; i++) {
        bool digit = isdigit((unsigned char)s[i]);
        if (digit && !in_number) {
            groups++;               // a new number starts here
        }
        in_number = digit;
    }
    return groups;
}
```

`count_digit_groups("tel 070 12")` is `2`. The flag's **starting value** describes the imaginary character *before* the first one. Ask yourself what that should be: is the start of the string like a digit, or like a space?

### Debugging: compare a passing and a failing input

The bug report gives you a gift: one input that works and one that doesn't, differing in a single place. Trace **both** by hand, side by side, writing the flag and the counter after every character. The first row where the two traces behave differently points straight at the code that handles that difference.
