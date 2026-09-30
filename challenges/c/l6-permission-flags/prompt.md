A file-sharing service stores each user's rights to a folder as a set of **bit flags** in one `unsigned`:

```c
typedef enum {
    PERM_READ  = 1u << 0,
    PERM_WRITE = 1u << 1,
    PERM_EXEC  = 1u << 2,
    PERM_ALL   = PERM_READ | PERM_WRITE | PERM_EXEC
} Perm;

unsigned perm_grant(unsigned perms, unsigned flags);
unsigned perm_revoke(unsigned perms, unsigned flags);
bool     perm_has_all(unsigned perms, unsigned required);
bool     perm_has_any(unsigned perms, unsigned wanted);
void     perm_to_string(unsigned perms, char out[4]);
```

- `perm_grant` returns `perms` with the `flags` added. Bits that aren't a known permission (outside `PERM_ALL`) are **dropped** from the result.
- `perm_revoke` returns `perms` with the `flags` removed. Everything else stays.
- `perm_has_all` is `true` if **every** flag in `required` is present. Requiring nothing (`0`) is always satisfied.
- `perm_has_any` is `true` if **at least one** flag in `wanted` is present.
- `perm_to_string` writes a Unix-style label such as `"rw-"` into `out` (3 letters plus `'\0'`).

```c
unsigned p = perm_grant(0, PERM_READ | PERM_WRITE);   // → 0x3
perm_has_all(p, PERM_READ | PERM_EXEC);                // → false
perm_has_any(p, PERM_READ | PERM_EXEC);                // → true
char label[4];
perm_to_string(perm_revoke(p, PERM_READ), label);      // → "-w-"
```

The enum is already in the starter.
