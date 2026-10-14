**Bug report from a subtitler:** "The editor says my caption has 11 words when I've clearly typed 12, so it lets me go over the limit. Oddly, if I start the caption with a space the count is right."

`caption_word_count` counts the words in a subtitle line:

```c
#include <stddef.h>

size_t caption_word_count(const char *text);
```

- A word is a run of one or more non-whitespace characters (use `isspace`).
- Any amount of whitespace (spaces, tabs, newlines) can sit between, before or after words.
- An empty or all-whitespace caption has `0` words.

What it should do:

- `"  where are you  "` → `3`
- `"wait\tfor\nme"` → `3`
- `""` → `0`

**Reproduction:** `caption_word_count("I know")` returns `1`, expected `2`.

Find the bug and fix it.
