A config loader needs to look up settings by name, quickly. Build a small hash map from strings to `int`s. The type is **opaque**: the tests only ever use it through these functions.

```c
typedef struct StrMap StrMap;

StrMap *strmap_new(void);
bool    strmap_put(StrMap *m, const char *key, int value);
bool    strmap_get(const StrMap *m, const char *key, int *out);
size_t  strmap_size(const StrMap *m);
void    strmap_free(StrMap *m);
```

- `strmap_new` returns an empty map, or `NULL` if allocation fails.
- `strmap_put` inserts `key → value`. If `key` already exists, **overwrite** its value (the size doesn't change). The map stores its **own copy** of the key, since the caller's string may change or disappear. It returns `false` only on allocation failure.
- `strmap_get` returns `true` and stores the value in `*out` if `key` is present. Otherwise it returns `false` and leaves `*out` unchanged.
- `strmap_size` returns the number of distinct keys.
- `strmap_free` releases **everything** the map allocated. `strmap_free(NULL)` does nothing.
- Keys are compared by content. The empty string is a valid key.

```c
StrMap *cfg = strmap_new();
strmap_put(cfg, "baud", 9600);
strmap_put(cfg, "baud", 115200);   // overwrite
int v;
strmap_get(cfg, "baud", &v);       // → true, v = 115200
strmap_get(cfg, "parity", &v);     // → false
strmap_size(cfg);                  // → 1
strmap_free(cfg);
```

The starter already has the layout (64 buckets, each a chain of `Entry` nodes), an FNV-1a `bucket_of(key)`, `strmap_new` and `strmap_size`. You write `strmap_put`, `strmap_get` and `strmap_free`. The tests insert 1000 keys, and the leak check runs after every test.
