A sensor pipeline applies different processing steps to a batch of readings: calibration offsets, clamping, totals, peaks. Instead of writing a loop for each one, write two reusable functions that take the step **as a function pointer**:

```c
void map_ints(int *arr, size_t n, int (*f)(int));
int  fold_ints(const int *arr, size_t n, int init, int (*combine)(int, int));
```

- `map_ints` replaces **every** element with `f(element)`, in place. It calls `f` exactly once per element, from first to last.
- `fold_ints` combines the elements into one value, left to right: it starts with `acc = init`, then sets `acc = combine(acc, arr[i])` for each element, and returns `acc`.
- With `n == 0`, `map_ints` does nothing and `fold_ints` returns `init`. Neither calls the function pointer.

```c
static int twice(int x) { return 2 * x; }
static int add(int a, int b) { return a + b; }

int r[] = {1, 2, 3};
map_ints(r, 3, twice);           // r → {2, 4, 6}
fold_ints(r, 3, 0, add);         // → 12
fold_ints(r, 0, 99, add);        // → 99
```

The test file defines the callbacks it passes in.
