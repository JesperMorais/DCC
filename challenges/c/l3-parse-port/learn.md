### From characters to a number

Digits are stored as character codes, and `'0'`–`'9'` are consecutive, so `c - '0'` gives the digit's value. To build a number, shift what you have one decimal place left and add the new digit:

```
"472":  0 → 0*10+4 = 4 → 4*10+7 = 47 → 47*10+2 = 472
```

### Validate as you go

A parser should decide *while* it reads, not afterwards. Here's one for a two-letter country code like `"SE"`:

```c
bool parse_country(const char *s, char code[3]) {
    for (int i = 0; i < 2; i++) {
        if (!isupper((unsigned char)s[i])) return false;  // also stops at '\0'
    }
    if (s[2] != '\0') return false;                        // trailing junk
    code[0] = s[0]; code[1] = s[1]; code[2] = '\0';
    return true;
}
```

It only writes the output once the whole input has been accepted.

### Overflow sneaks up on you

An `unsigned` holds at most 4 294 967 295. Keep multiplying by 10 past that and the value silently **wraps around** to a small number, which might look perfectly valid. The fix is to check your limit after every digit, while the number is still small enough to be trustworthy.
