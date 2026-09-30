### What `const` promises

`const` on a pointer's target means "I won't modify what this points at":

```c
void print_name(const char *name);   // reads name, never writes it
void shout(char *text);              // may modify text in place
```

The compiler enforces it. Writing `name[0] = 'X';` inside `print_name` is a compile error. And the promise is what lets callers pass read-only data: a `const char *` can go to a `const char *` parameter, but handing it to a plain `char *` parameter triggers a warning, because the function *might* write to it.

### Where `const` goes

Read declarations right to left:

```c
const char *p;        // p is a pointer to const char: can't change *p, can move p
char *const q = buf;  // q is a const pointer to char: can change *q, can't move q
```

The first one is what you want almost every time for input strings.

### const flows through

If you return a pointer **into** a const input, the result has to be const as well, otherwise you've quietly handed out write access to something that was promised to stay unchanged:

```c
const int *largest(const int *a, size_t n);   // honest
int *largest(const int *a, size_t n);         // would need a cast inside: a lie
```

The standard `strchr` is a famous wart here: it takes `const char *` but returns `char *`, for historical reasons. Don't copy that design.
