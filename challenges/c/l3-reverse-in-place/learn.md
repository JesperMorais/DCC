### Two indices moving toward each other

Plenty of array problems are solved by one index starting at the front and another at the back, both stepping inward until they meet. For example, this checks whether an array of numbers reads the same both ways:

```c
bool is_mirror(const int *a, size_t n) {
    if (n == 0) return true;
    for (size_t lo = 0, hi = n - 1; lo < hi; lo++, hi--) {
        if (a[lo] != a[hi]) return false;
    }
    return true;
}
```

### Swapping needs a temporary

To exchange two values you have to park one of them somewhere first:

```c
int tmp = x;
x = y;
y = tmp;
```

### The `size_t` trap

`size_t` is unsigned: it can't go below zero. `0 - 1` doesn't give `-1`, it **wraps around** to the largest possible `size_t` (about 18 quintillion). That's why `is_mirror` above handles `n == 0` before it computes `n - 1`. Using that wrapped value as an index is a classic out-of-bounds bug.
