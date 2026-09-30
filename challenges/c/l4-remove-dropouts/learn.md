### The read/write two-pointer technique

When you want to filter an array in place, walk it with two positions:

- a **read** position that visits every element, and
- a **write** position that only moves forward when you keep something.

Because you keep at most one element per element read, the write position can never overtake the read position, so you never overwrite something you haven't looked at yet.

Here's the same technique used to squeeze repeated values out of a sorted array, like `{1, 1, 2, 3, 3}` → `{1, 2, 3}`:

```c
size_t dedupe_sorted(int *a, size_t n) {
    if (n == 0) return 0;
    size_t w = 1;
    for (size_t r = 1; r < n; r++) {
        if (a[r] != a[w - 1]) {
            a[w] = a[r];
            w++;
        }
    }
    return w;
}
```

Returning the new length is the C way to "shrink" an array: the memory stays the same size, the caller just stops looking past the new end. It's a single pass and uses no extra memory, which beats both "copy into a new array" and "shift everything left each time you delete" (that one is O(n²)).
