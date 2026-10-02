#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POOL_BLOCK_SIZE 32
#define POOL_BLOCKS 8

/* Your static storage and bookkeeping go here. */

void pool_init(void) {
}

void *pool_alloc(void) {
    return NULL;
}

bool pool_free(void *block) {
    (void)block;
    return false;
}

size_t pool_free_count(void) {
    return 0;
}
