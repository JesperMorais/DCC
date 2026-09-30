A data logger doesn't know how many samples it will record, so it stores them in a growable array.

```c
typedef struct {
    int *data;    // heap block, or NULL while empty
    size_t len;   // how many values are stored
    size_t cap;   // how many values fit before we must grow
} IntVec;

void intvec_init(IntVec *v);
bool intvec_push(IntVec *v, int value);
int  intvec_get(const IntVec *v, size_t index);
void intvec_free(IntVec *v);
```

- `intvec_init` sets up an empty vector: `data = NULL`, `len = 0`, `cap = 0`. No allocation yet.
- `intvec_push` appends `value`. When the vector is full, grow it with `realloc`: the capacity goes **0 → 4 → 8 → 16 → …** (start at 4, then double). Return `false` if the allocation fails, leaving the vector as it was.
- `intvec_get` returns the value at `index`. The caller guarantees `index < len`.
- `intvec_free` frees the storage and leaves `v` as an **empty vector again**, so it can be reused.

```c
IntVec v;
intvec_init(&v);
intvec_push(&v, 10);      // len 1, cap 4
intvec_push(&v, 20);      // len 2, cap 4
intvec_get(&v, 1);        // → 20
intvec_free(&v);          // data NULL, len 0, cap 0
```
