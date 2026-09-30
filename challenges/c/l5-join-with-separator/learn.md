### Measure, allocate, fill

C won't grow a string for you, so building one follows a fixed rhythm:

1. **Measure** how many bytes the result needs.
2. **Allocate** that many, once.
3. **Fill** the buffer, keeping track of where the next byte goes.

```c
// "[" + text + "]"
size_t n = strlen(text);
char *out = malloc(1 + n + 1 + 1);   // '[' + text + ']' + '\0'
if (out == NULL) return NULL;
char *p = out;          // write cursor
*p++ = '[';
memcpy(p, text, n);     // memcpy doesn't add a '\0'
p += n;
*p++ = ']';
*p = '\0';
```

Writing the size as a sum of named pieces, as above, makes off-by-one mistakes much easier to spot than a bare `n + 3`.

### Why exact matters

A buffer one byte too short usually "works" on your machine, because `malloc` tends to round sizes up, until the day it corrupts the next allocation. AddressSanitizer tracks the exact size you asked for, so it catches this immediately.

### `strcat` in a loop

`strcat(out, part)` has to scan `out` from the start to find its end every time, which makes a loop of them O(n²). Keeping your own cursor avoids that.
