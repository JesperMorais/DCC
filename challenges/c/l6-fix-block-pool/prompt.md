**Bug report from the firmware team**

> The packet-buffer pool has been running on the board for months. Since we turned on UBSan in CI, any test that allocates more than a handful of blocks dies with a runtime error inside the pool. Nobody has touched that code.

`BlockPool` hands out `POOL_BLOCKS` (64) fixed-size blocks, tracking them with one bit per block in an array of `uint32_t` words (block `b` is bit `b % 32` of word `b / 32`).

- `pool_init` marks every block free.
- `pool_alloc` returns the **lowest-numbered** free block and marks it used, or `-1` if none are free.
- `pool_free(pool, b)` marks block `b` free and returns `true`. Freeing a block that isn't allocated, or that doesn't exist, returns `false` and changes nothing.
- `pool_is_used(pool, b)` tells whether block `b` is allocated (`false` for blocks that don't exist).

```c
BlockPool pool;
pool_init(&pool);
pool_alloc(&pool);       // 0
pool_alloc(&pool);       // 1
pool_free(&pool, 0);     // true
pool_alloc(&pool);       // 0 again: lowest free block
pool_free(&pool, 40);    // false: never allocated
```

**Reproduction**

```c
BlockPool pool;
pool_init(&pool);
for (int i = 0; i < 32; i++) pool_alloc(&pool);
// actual:   the 32nd call stops with a UBSan runtime error
// expected: returns 0, 1, ..., 31
```
