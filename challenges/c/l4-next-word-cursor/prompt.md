A tiny shell for a microcontroller splits commands like `"  led  set 3 "` into words, **without copying anything** (there's barely any RAM). Write `next_word`:

```c
bool next_word(const char **cursor, const char **word, size_t *len);
```

`*cursor` is the caller's current position in the text. Each call:

1. skips any spaces (`' '`) at `*cursor`,
2. if it reaches the end of the string, sets `*cursor` to point at the `'\0'` and returns `false`,
3. otherwise sets `*word` to the start of the word and `*len` to its length, **moves `*cursor` to just after the word**, and returns `true`.

So the caller can loop:

```c
const char *cur = "  led  set 3 ";
const char *w;
size_t n;
while (next_word(&cur, &w, &n)) {
    printf("[%.*s]\n", (int)n, w);   // [led] [set] [3]
}
```

`*word` points into the original text (it isn't terminated at the word's end, which is why `*len` exists).
