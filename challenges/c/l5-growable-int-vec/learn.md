### `realloc` resizes a heap block

```c
double *buf = malloc(8 * sizeof *buf);
double *bigger = realloc(buf, 32 * sizeof *bigger);
```

`realloc` either grows the block where it is or moves it: it allocates a new block, copies the old contents over and frees the old one. Either way you must use the **returned** pointer from then on. Two details catch people out:

- **Sizes are bytes.** Asking for `32` gives room for 32 bytes, not 32 doubles. `sizeof *ptr` is the size of one element and stays correct even if the type changes later.
- **It can fail.** On failure it returns `NULL` and the old block is left alone. Writing `buf = realloc(buf, n)` throws away your only pointer to that old block, which is a leak. Keep the result in a temporary until you know it worked.

`realloc(NULL, n)` is the same as `malloc(n)`.

### Why double?

Growing by one slot each time copies the whole array on every push, which is O(n²) in total. Doubling means the copies add up to less than 2n over n pushes, so each push costs O(1) *on average* ("amortized"). Most real vectors grow this way, including C++'s `std::vector` and Rust's `Vec`.
