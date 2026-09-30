### From key to slot in O(1)

A hash table turns a key into an array index with a **hash function**: a fast computation that spreads different keys across the array.

```c
size_t slot = hash(key) % BUCKET_COUNT;
```

Two different keys can land in the same slot. That's a **collision**, and every hash table needs a plan for it. With *separate chaining*, each slot holds a small linked list, and lookups walk only that list, comparing keys with `strcmp`. As long as the lists stay short, `get` and `put` are O(1) on average. (Production tables grow the bucket array when they get too full. A fixed size is fine for this kata.)

A good simple string hash is **FNV-1a**. Mix in each byte, then multiply by a prime:

```c
uint32_t h = 2166136261u;
for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
    h ^= *p;
    h *= 16777619u;   // unsigned: wrapping is well-defined
}
```

### Opaque types

`typedef struct StrMap StrMap;` without a body declares an *incomplete type*: users can hold `StrMap *` pointers but can't see or touch the fields. That's C's version of a private class. You can change the internals without breaking any caller.

### Who owns the keys?

If the map only stored the caller's pointer, a key built in a reused buffer would silently change inside the map. Copying the key means the map **owns** that copy, so the map has to free it too.
