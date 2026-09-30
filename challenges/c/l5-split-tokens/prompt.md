A tiny shell needs to turn the line the user typed into an `argv`-style array of words:

```c
char **split_tokens(const char *line, char delim, size_t *count);
void   free_tokens(char **tokens);
```

- Split `line` on `delim`. Runs of delimiters count as **one**, and leading or trailing delimiters produce **no** empty tokens.
- Return a `malloc`'d array of `malloc`'d strings, one allocation **per token**, followed by a final `NULL` entry (just like `argv`). Store the number of tokens in `*count`.
- A line with no tokens (empty, or only delimiters) returns an array holding just `NULL`, with `*count == 0`.
- Don't modify `line` (it may be a string literal).
- `free_tokens` frees every token **and** the array. `free_tokens(NULL)` does nothing.
- Return `NULL` if an allocation fails, without leaking what you had built so far.

```c
size_t n;
char **argv = split_tokens("  ls   -la /tmp ", ' ', &n);
// n == 3, argv = {"ls", "-la", "/tmp", NULL}
free_tokens(argv);
```

Because each token is its own allocation, the caller may `free` one token and replace it with a string of their own before calling `free_tokens`.
