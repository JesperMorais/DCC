### Returning a pointer

A function can hand back an **address** instead of a value. That's how the standard library's search functions work: `strchr` returns a pointer to the character it found, `bsearch` a pointer to the matching element. The caller gets more than the value: it knows *where* it is, and can change it.

```c
double *find_max_price(double *prices, size_t n) {
    if (n == 0) return NULL;
    double *best = &prices[0];
    for (size_t i = 1; i < n; i++) {
        if (prices[i] > *best) best = &prices[i];
    }
    return best;
}
```

### `NULL` means "nothing here"

`NULL` is a pointer that points nowhere. Returning it is the conventional way to say "not found", and callers are expected to check before using the result:

```c
double *p = find_max_price(prices, n);
if (p != NULL) *p *= 0.9;   // 10% off the priciest item
```

Dereferencing `NULL` crashes your program, so it's never a valid "real" answer.

### Pointer subtraction

Subtracting two pointers into the same array gives the distance between them in elements: `p - prices` is the index `p` points at.

**Never return the address of a local variable.** It stops existing when the function returns. Pointers into the *caller's* array are fine, because the caller still owns that memory.
