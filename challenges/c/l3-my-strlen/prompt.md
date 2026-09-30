An SMS gateway rejects messages longer than 160 characters, and you need to know how long each message is before sending it. Write `my_strlen`, which returns the number of characters in a C string, **not counting** the terminating `'\0'`:

```c
size_t my_strlen(const char *s);
```

- `my_strlen("hello")` → `5`
- `my_strlen("")` → `0`
- `my_strlen("see you at 8")` → `12` (spaces count)

Don't call `strlen` from `<string.h>`. The point is to find the terminator yourself, and to stop there: reading even one byte past it is a bug.
