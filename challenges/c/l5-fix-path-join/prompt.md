**Bug report from CI**

> The new config loader crashes in the sanitizer build with a heap-buffer-overflow in `path_join`. It doesn't happen for every path, and the release build seems fine, which is somehow worse.

`path_join` builds `dir + "/" + name` in a newly `malloc`ed string that the caller frees:

- Exactly one `/` goes between the two parts: it's added only if `dir` doesn't already end with one.
- An empty `dir` gives a copy of `name`, with no slash.
- It returns `NULL` only if `malloc` fails.

```c
path_join("logs", "app.txt")     // "logs/app.txt"
path_join("/var/log/", "syslog") // "/var/log/syslog"
path_join("", "notes.md")        // "notes.md"
```

**Reproduction**

```c
char *p = path_join("logs", "app.txt");
// actual:   AddressSanitizer: heap-buffer-overflow
// expected: "logs/app.txt"
```
