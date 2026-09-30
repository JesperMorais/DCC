### Strings are char arrays with an end marker

C has no string type. A string is an array of `char` whose last element is the **null terminator** `'\0'` (the byte with value 0):

```c
char word[] = "cat";   // really {'c', 'a', 't', '\0'}: 4 bytes
```

The length isn't stored anywhere. Every string function in C works by walking forward until it reaches `'\0'`, so if the terminator is missing, the function keeps reading into memory that isn't yours.

### Walking with an index

Here's a loop that stops at the terminator and asks a question about each character on the way:

```c
bool contains_space(const char *s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == ' ') return true;
    }
    return false;
}
```

`const char *` means "a pointer to characters I promise not to change". `size_t` is the unsigned type C uses for sizes and lengths.
