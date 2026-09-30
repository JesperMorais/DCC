### Changing a string in place

A string literal like `"hello"` lives in read-only memory: writing to it crashes. To get a string you *can* change, copy it into an array:

```c
char name[] = "alice";   // a 6-byte array you own
name[0] = 'A';           // fine: name is now "Alice"
```

A function that takes `char *` (no `const`) is telling you it may modify the characters. It works on the caller's array directly, so nothing needs to be returned.

### Remembering what came before

Many string tasks depend on the *previous* character, not just the current one. A small `bool` flag carries that state through the loop. For example, here's a function that squeezes runs of `-` into one:

```c
void squeeze_dashes(char *s) {
    size_t out = 0;
    bool prev_dash = false;
    for (size_t i = 0; s[i] != '\0'; i++) {
        bool dash = s[i] == '-';
        if (!(dash && prev_dash)) s[out++] = s[i];
        prev_dash = dash;
    }
    s[out] = '\0';
}
```

`toupper`/`tolower` return an `int`, so cast back when storing: `s[i] = (char)toupper((unsigned char)s[i]);`.
