A file server picks a `Content-Type` from each upload's extension. A colleague started on the helper, but its signature is wrong: it takes and returns a plain `char *`, so it can't be called with a `const char *` path without a warning, and it hands callers a pointer they could write through.

Fix it so it's **const-correct**, and implement it:

```c
const char *file_extension(const char *path);
```

- Return a pointer **into `path`**, to the character just after the **last** `'.'`.
- If there's no `'.'` at all, return `NULL`.
- If the `'.'` is the last character, return a pointer to the (empty) rest of the string, i.e. to the `'\0'`.
- Don't copy the string and don't cast away `const`.

```c
file_extension("report.pdf")       // → "pdf"
file_extension("backup.tar.gz")    // → "gz"
file_extension("Makefile")         // → NULL
file_extension("draft.")           // → ""
```

The tests check the exact signature, so keep the name.
