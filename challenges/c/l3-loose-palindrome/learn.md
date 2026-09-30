### Skipping what you don't care about

A common pattern when scanning text: before looking at a character, skip past the ones that don't matter. The important part is that the skipping loop **also checks the boundary**, so it can't run off the end of the data.

Here's a function that finds where the next number starts in a string, or returns the length if there isn't one:

```c
size_t next_digit(const char *s, size_t from, size_t len) {
    size_t i = from;
    while (i < len && !isdigit((unsigned char)s[i])) {
        i++;
    }
    return i;
}
```

Notice the order inside the condition: `i < len` is checked **first**. `&&` short-circuits, so once `i` reaches `len`, `s[i]` is never read.

### Case-insensitive comparison

To compare two letters ignoring case, normalise both first:

```c
bool same_letter(char a, char b) {
    return tolower((unsigned char)a) == tolower((unsigned char)b);
}
```
