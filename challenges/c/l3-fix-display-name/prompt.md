**Bug report from the forum admin:** "Someone logged in as `administrator`, logged out, and then `bob` logged in on the same terminal. The header said *Welcome, bobinistrator*. New sessions look fine."

`copy_display_name` copies a user's name into a fixed-size buffer that the header is drawn from:

```c
#include <stddef.h>

void copy_display_name(char *dst, const char *src, size_t cap);
```

- `cap` is the size of `dst` in bytes, and is always at least `1`.
- Copy as much of `src` as fits, leaving room for the terminator: at most `cap - 1` characters.
- `dst` must **always** end up holding a proper string (ending in `'\0'`), whatever it held before.

What it should do:

```c
char header[16] = {0};
copy_display_name(header, "ada", 16);          // header is "ada"

char tiny[6] = {0};
copy_display_name(tiny, "alexandra", 6);       // tiny is "alexa" (5 chars + '\0')
```

**Reproduction:**

```c
char header[16] = "administrator";
copy_display_name(header, "bob", 16);
// header is "bobinistrator", expected "bob"
```

Find the bug and fix it.
