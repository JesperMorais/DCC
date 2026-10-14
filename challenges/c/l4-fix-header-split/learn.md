### Splitting a string without copying it

C code often parses text **in place**: find a separator, overwrite it with `'\0'`, and hand out pointers into the original buffer. Nothing is allocated, and both halves are ordinary C strings:

```c
char pair[] = "key=value";
char *eq = strchr(pair, '=');   // pointer to the '=', or NULL
*eq = '\0';                     // pair now reads "key"
char *val = eq + 1;             // "value"
```

The pointers are only valid while the buffer is, which is the price of not copying.

`<string.h>` has a family of search functions that return a pointer into the string (or `NULL`):

| function | finds |
|---|---|
| `strchr(s, c)` | the first `c` |
| `strrchr(s, c)` | the last `c` |
| `strstr(s, t)` | the first occurrence of string `t` |

### Debugging: hunt for the input that breaks it

The report says "most headers are fine". Collect a few that work and a few that don't, and ask what the failing ones have in common that the working ones lack. Shrinking the failing input to the smallest version that still fails usually points straight at the rule the code gets wrong.
