A 3D printer's firmware reads G-code such as `G1X120Y-45F3000`: letters followed by numbers, with nothing in between. Write the number parser it needs, `strtol`-style, where an **end pointer** tells the caller where the number stopped:

```c
bool parse_int_prefix(const char *s, int *out, const char **end);
```

- Accepted format: an optional single `+` or `-`, then **one or more** decimal digits. Parsing stops at the first character that isn't a digit. That character (or the `'\0'`) is not an error, it's where the caller continues.
- **Success:** store the value in `*out`, point `*end` at the first unparsed character, return `true`.
- **Failure:** there are no digits, or the value doesn't fit in an `int`. Return `false`, leave `*out` unchanged, and set `*end = s` (nothing was consumed).
- No whitespace skipping: `" 5"` is a failure.
- Don't call `strtol`, `atoi` or `sscanf`, and never overflow (UBSan is watching).

```c
const char *line = "X120Y-45";
const char *rest;
int x, y;
parse_int_prefix(line + 1, &x, &rest);   // → true, x = 120, rest → "Y-45"
parse_int_prefix(rest + 1, &y, &rest);   // → true, y = -45, rest → ""
parse_int_prefix("F", &x, &rest);        // → false, rest → "F", x unchanged
parse_int_prefix("2147483648", &x, &rest); // → false: too big for int
```
