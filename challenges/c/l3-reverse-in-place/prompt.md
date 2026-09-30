A tiny embedded display scrolls text right-to-left, and its driver wants each label reversed. There's no spare memory for a second buffer, so write `reverse_in_place`, which reverses a string **inside its own array**:

```c
void reverse_in_place(char *s);
```

```c
char label[] = "READY";
reverse_in_place(label);   // label is now "YDAER"
```

- `"ab"` → `"ba"`
- `"a"` → `"a"`
- `""` → `""`

The terminator must stay at the end. You may use `strlen` from `<string.h>`.
