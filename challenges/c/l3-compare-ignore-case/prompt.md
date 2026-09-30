A chat app sorts its member list alphabetically, and `"alice"`, `"Alice"` and `"ALICE"` should sort together. Write `compare_ignore_case`, which compares two strings the way `strcmp` does, but **ignoring case**:

```c
int compare_ignore_case(const char *a, const char *b);
```

It returns:

- a **negative** number if `a` sorts before `b`,
- **0** if they're equal apart from case,
- a **positive** number if `a` sorts after `b`.

Only the sign matters. Compare letter by letter; a string that is a **prefix** of the other sorts first.

- `compare_ignore_case("Alice", "aLiCe")` → `0`
- `compare_ignore_case("bob", "Carol")` → negative
- `compare_ignore_case("zoe", "Zack")` → positive
- `compare_ignore_case("ann", "Anna")` → negative (prefix)

Don't use `strcmp`, `strcasecmp` or `strncasecmp`.
