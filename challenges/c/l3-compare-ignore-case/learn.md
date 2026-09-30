### How `strcmp` thinks

`strcmp(a, b)` walks both strings in step. At the first position where the characters differ, it returns their difference: negative if `a`'s char is smaller, positive if it's bigger. If it reaches the end of both at once, the strings are equal and it returns 0.

The neat trick is the terminator. `'\0'` has the value 0, smaller than any real character, so a string that runs out first automatically compares as smaller: `"car" < "cart"`.

### Walking two sequences at once

Here's the same idea on arrays of version numbers, like `{1, 4, 2}` versus `{1, 10}`:

```c
int compare_versions(const int *a, size_t na, const int *b, size_t nb) {
    size_t i = 0;
    while (i < na && i < nb) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
        i++;
    }
    if (na == nb) return 0;
    return na < nb ? -1 : 1;   // the shorter one is a prefix
}
```

With strings you don't need separate lengths: stopping at `'\0'` does that job, **as long as you stop there** instead of reading past it.
