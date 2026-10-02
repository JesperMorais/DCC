Your sensor firmware passes 32-byte messages between its drivers and the radio. The safety reviewer's rule is **no `malloc`, ever**. Build a fixed-block pool on static storage:

```c
#define POOL_BLOCK_SIZE 32
#define POOL_BLOCKS 8

void   pool_init(void);
void  *pool_alloc(void);
bool   pool_free(void *block);
size_t pool_free_count(void);
```

- **`pool_init`** puts the pool in its "all free" state. Tests may call it more than once, and every call must reset the pool completely.
- **`pool_alloc`** returns a block of `POOL_BLOCK_SIZE` bytes that nobody else is using. It must be **8-byte aligned**, so it can hold a `uint64_t` or a `double`. When all `POOL_BLOCKS` blocks are taken, it returns `NULL`. It must not crash or overwrite anything.
- **`pool_free`** gives a block back and returns `true`. It returns `false`, and changes **nothing**, when the pointer is:
  - `NULL`
  - not a block from this pool (a stack variable, or a pointer into the *middle* of a block)
  - a block that's already free (a **double free**)
- **`pool_free_count`** returns how many blocks are free right now.
- The memory must be **static**: no `malloc`, `calloc` or VLAs. (The leak checker runs after every test, so heap memory that's never freed fails the test.)

```c
pool_init();
uint8_t *a = pool_alloc();       // a 32-byte block
uint8_t *b = pool_alloc();       // a different one
pool_free_count();               // → 6
pool_free(a);                    // → true
pool_free(a);                    // → false: double free caught
pool_free(b + 4);                // → false: not the start of a block
```

A freed block may be handed out again by a later `pool_alloc`, and that's the point of a pool.
