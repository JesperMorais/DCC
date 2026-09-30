### Pointer arithmetic

Adding 1 to a pointer doesn't add one *byte*. It moves to the **next element**, however big that element is:

```c
double prices[] = {1.5, 2.25, 9.0};
const double *p = prices;   // points at prices[0]
p++;                        // now at prices[1]
double x = *(p + 1);        // prices[2], i.e. 9.0
```

In fact `a[i]` is defined as `*(a + i)`. Indexing is pointer arithmetic with nicer syntax.

### Half-open ranges

Instead of "pointer + count", a lot of C (and C++) code describes a range with two pointers: where it **begins**, and one position **past** where it ends. `end` is a legal address to compute and compare, but not to dereference.

```c
size_t count_zeros(const double *begin, const double *end) {
    size_t n = 0;
    for (const double *p = begin; p != end; p++) {
        if (*p == 0.0) n++;
    }
    return n;
}
```

Half-open ranges compose nicely: the empty range is `begin == end`, and splitting `[a, c)` at `b` gives `[a, b)` and `[b, c)` with nothing counted twice.
