### Why a pointer to a pointer?

To let a function change the caller's `int`, you pass an `int *`. The same logic applies one level up: to let a function change the caller's **pointer**, you pass a pointer *to* that pointer.

```c
void skip_digits(const char **s) {
    while (isdigit((unsigned char)**s)) {
        (*s)++;          // move the caller's pointer forward
    }
}

const char *p = "2026-09-30";
skip_digits(&p);         // p now points at "-09-30"
```

Reading the types:

| Expression | Type | Meaning |
|---|---|---|
| `s` | `const char **` | address of the caller's pointer |
| `*s` | `const char *` | the caller's pointer itself |
| `**s` | `const char` | the character it points at |

Note the parentheses in `(*s)++`: without them, `*s++` would move `s`, not `*s`.

### Work on a local copy

Double dereferences get hard to read quickly. A common style is to copy the pointer in, work with it, and write it back once at the end:

```c
void skip_digits(const char **s) {
    const char *p = *s;
    while (isdigit((unsigned char)*p)) p++;
    *s = p;
}
```

This is exactly how `strtol`'s `char **endptr` tells you where parsing stopped.
