### Pointers are addresses

Every variable lives at an address in memory. `&x` gives you the address of `x`, and a pointer (`int *p`) stores one:

```c
int x = 5;
int *p = &x;   // p points at x
*p = 42;       // write through the pointer: x is now 42
```

`*p` means "the thing `p` points at".

### Out-parameters

C functions return a single value. The idiomatic way to give back several is to let the caller pass addresses to fill in, and to use the return value for "did it work?":

```c
bool divide(int a, int b, int *quotient) {
    if (b == 0) return false;   // refuse, and don't touch *quotient
    *quotient = a / b;
    return true;
}

int q;
if (divide(10, 3, &q)) { /* q == 3 */ }
```

You'll see this pattern all over real C APIs, like `strtol` and `fread`.

### Arrays decay to pointers

When you pass an array to a function, the function receives a pointer to its first element. It has **no idea how long the array is**, which is why the length travels alongside as a separate `size_t` parameter.
