### Who owns the memory?

A C function that "returns a string" usually doesn't. Instead, the **caller** hands over a buffer and says how big it is, and the function fills it in. You'll see this shape everywhere: `snprintf(buf, size, ...)`, `strftime`, `getcwd`.

```c
char buf[16];
format_time(buf, sizeof buf, seconds);
```

The contract is strict: the function **must not write outside** the `size` bytes it was given, and the result must be terminated with `'\0'`. That means only `size - 1` bytes of actual text fit.

### Checking before writing

Here's a function that copies at most one line into a buffer, refusing if it's too long:

```c
bool copy_line(const char *src, char *dst, size_t cap) {
    if (cap == 0) return false;           // not even room for '\0'
    size_t n = 0;
    while (src[n] != '\0' && src[n] != '\n') {
        if (n + 1 >= cap) return false;   // no room for this char AND the '\0'
        dst[n] = src[n];
        n++;
    }
    dst[n] = '\0';
    return true;
}
```

The check `n + 1 >= cap` asks "would writing this character leave room for the terminator?" Getting it off by one is one of the most common security bugs in C: the buffer overflow.
