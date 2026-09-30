### Arrays of pointers

A `char **` is a pointer to the first element of an array whose elements are `char *`. In this layout every string is a separate heap block, and the array holding the pointers is one more block:

```
names ──► [ ptr ][ ptr ][ NULL ]
             │      │
             ▼      ▼
          "ada\0" "grace\0"
```

That's three allocations, so it takes three `free`s. Free the strings **first**: once the array is gone you can no longer read the pointers stored in it.

```c
char **names = malloc(3 * sizeof *names);   // room for 3 pointers
names[0] = malloc(4);  memcpy(names[0], "ada", 4);
/* ... */
for (size_t i = 0; names[i] != NULL; i++) free(names[i]);
free(names);
```

### A sentinel instead of a length

`argv` ends with a `NULL` entry, so code can loop without knowing the count, much like `'\0'` ends a string. Remember to allocate room for that sentinel.

### Clean up on failure

A function that makes several allocations must decide what happens when the fifth one fails: the first four are still live. The robust pattern is to release everything built so far and report failure, so the caller never ends up with half an object. Pairing each "make" function with a matching "free" function keeps that knowledge in one place.
