### The heap and who owns it

Local variables live on the **stack** and vanish when the function returns. Memory from `malloc` lives on the **heap** until someone calls `free` on it:

```c
int *make_counter(void) {
    int *c = malloc(sizeof *c);   // room for one int
    if (c == NULL) return NULL;   // malloc can fail
    *c = 0;
    return c;                     // the caller now owns it
}

int *c = make_counter();
free(c);                          // exactly once, when you're done
```

The rule that keeps C programs sane is **ownership**: every allocation has exactly one owner, the one responsible for freeing it. A function that returns `malloc`'d memory should say so in its documentation. Forget the `free` and you leak; free twice and you corrupt the heap.

### Sizes are in bytes

`malloc(n)` gives you `n` bytes, not `n` "things". For a string that means thinking about every byte it occupies, not just the visible characters. AddressSanitizer is watching: writing even one byte past the end of a block fails the test.

`<string.h>` has `strlen`, `memcpy` and `strcpy` to help.
