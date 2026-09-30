### Reporting *where*, not just *what*

A parser that only returns a number can't tell you whether `"12abc"` was a number, a typo or the start of something longer. The C library solves this with an **end pointer**: an out-parameter the function sets to the first character it didn't use.

```c
char *end;
long v = strtol("250ms", &end, 10);   // v = 250, end points at "ms"
if (end == input) { /* no number at all */ }
```

Callers can then check that the whole string was a number (`*end == '\0'`), or keep going from `end` to parse the next field. The parameter is a `const char **` because the function has to change the caller's `const char *` variable.

### Digits to value

Characters `'0'` to `'9'` have consecutive codes, so `c - '0'` is the digit's value and building a number is `value = value * 10 + digit`.

### The asymmetric range

`int` usually runs from `-2147483648` to `2147483647`. There's one more negative number than positive ones, so `"-2147483648"` is valid. But if you build `2147483648` first and negate it afterwards, the intermediate value has already overflowed. Think about which direction has room for every valid input.

As in any overflow check, test the limit **before** the operation that could exceed it, rearranging the inequality so the check itself can't overflow.
