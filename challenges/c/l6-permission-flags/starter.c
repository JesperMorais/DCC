#include <stdbool.h>

typedef enum {
    PERM_READ  = 1u << 0,
    PERM_WRITE = 1u << 1,
    PERM_EXEC  = 1u << 2,
    PERM_ALL   = PERM_READ | PERM_WRITE | PERM_EXEC
} Perm;

unsigned perm_grant(unsigned perms, unsigned flags) {
    (void)flags;
    return perms;
}

unsigned perm_revoke(unsigned perms, unsigned flags) {
    (void)flags;
    return perms;
}

bool perm_has_all(unsigned perms, unsigned required) {
    (void)perms;
    (void)required;
    return false;
}

bool perm_has_any(unsigned perms, unsigned wanted) {
    (void)perms;
    (void)wanted;
    return false;
}

void perm_to_string(unsigned perms, char out[4]) {
    (void)perms;
    out[0] = '\0';
}
