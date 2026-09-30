### C passes copies

When you call a function in C, it gets **copies** of the arguments. Changing a parameter changes the copy, and the caller never sees it:

```c
void reset(int n) { n = 0; }     // pointless: only the copy is reset

int score = 42;
reset(score);                    // score is still 42
```

### Pass the address instead

If the function receives the *address* of the caller's variable, it can reach back and change the original:

```c
void reset(int *n) { *n = 0; }

int score = 42;
reset(&score);                   // score is now 0
```

- `&score` means "the address of `score`".
- `int *n` declares a pointer to an `int`.
- `*n` (the *dereference*) is the `int` it points at, and you can read it or assign to it.

A pointer parameter can be passed straight on to another function that wants one, with no `&`: it's already an address.

```c
void clamp_to_zero(int *n) {
    if (*n < 0) reset(n);
}
```
