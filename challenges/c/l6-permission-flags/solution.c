#include <stdbool.h>

typedef enum {
    PERM_READ  = 1u << 0,
    PERM_WRITE = 1u << 1,
    PERM_EXEC  = 1u << 2,
    PERM_ALL   = PERM_READ | PERM_WRITE | PERM_EXEC
} Perm;

unsigned perm_grant(unsigned perms, unsigned flags) {
    return (perms | flags) & PERM_ALL;
}

unsigned perm_revoke(unsigned perms, unsigned flags) {
    return perms & ~flags;
}

bool perm_has_all(unsigned perms, unsigned required) {
    return (perms & required) == required;
}

bool perm_has_any(unsigned perms, unsigned wanted) {
    return (perms & wanted) != 0;
}

void perm_to_string(unsigned perms, char out[4]) {
    out[0] = (perms & PERM_READ) ? 'r' : '-';
    out[1] = (perms & PERM_WRITE) ? 'w' : '-';
    out[2] = (perms & PERM_EXEC) ? 'x' : '-';
    out[3] = '\0';
}
