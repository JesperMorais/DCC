You're writing the header row of a CSV export. Glue an array of strings together with a separator between them, into a **newly allocated** string:

```c
char *join_strings(const char *const *parts, size_t count, const char *sep);
```

- The separator goes **between** parts, never before the first or after the last.
- Allocate **exactly** the bytes the result needs, `'\0'` included. AddressSanitizer checks every write.
- With `count == 0`, return a newly allocated empty string `""`.
- Parts and separator may be empty strings.
- Return `NULL` only if `malloc` fails. The caller `free`s the result.

```c
const char *cols[] = {"time", "temp", "humidity"};
join_strings(cols, 3, ", ");   // → "time, temp, humidity"
join_strings(cols, 1, ", ");   // → "time"
join_strings(cols, 0, ", ");   // → ""
```
