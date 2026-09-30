A badge printer has room for a few characters, so it prints people's initials. Write `initials`, which writes the uppercase first letter of every word in `name` into the **caller's buffer** `out`, followed by `'\0'`:

```c
bool initials(const char *name, char *out, size_t cap);
```

- `cap` is the total size of `out` in bytes, **including** the terminator.
- Words are separated by one or more spaces (`' '`).
- If the initials plus the `'\0'` fit in `cap` bytes, write them and return `true`.
- If they don't fit, return `false`. **Never write past `out[cap - 1]`**, and never write anything at all when `cap` is `0`.

```c
char out[4];
initials("ada lovelace", out, sizeof out);          // true,  out is "AL"
initials("Grace Brewster Hopper", out, sizeof out); // true,  out is "GBH" (exact fit)
initials("Tim Berners Lee Jr", out, sizeof out);    // false: "TBLJ" needs 5 bytes
initials("   ", out, sizeof out);                   // true,  out is ""
```
