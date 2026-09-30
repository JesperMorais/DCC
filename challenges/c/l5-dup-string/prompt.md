A sensor driver hands you its device name in a temporary buffer that it reuses on the next read. To keep the name around, you need your **own copy** on the heap.

Write `my_strdup`:

```c
char *my_strdup(const char *s);
```

- Return a newly `malloc`'d string with the same contents as `s`. The caller owns it and will `free()` it.
- The copy must be independent: changing the original afterwards must not change the copy.
- If `s` is `NULL`, return `NULL` (and allocate nothing).

```c
char *name = my_strdup("imu-0");   // → "imu-0" (a different pointer)
char *none = my_strdup("");        // → "" (still malloc'd, still needs free)
my_strdup(NULL);                   // → NULL
```

Don't call the library's `strdup` — write it yourself.
