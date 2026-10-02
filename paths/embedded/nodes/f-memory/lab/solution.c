#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POOL_BLOCK_SIZE 32
#define POOL_BLOCKS 8

static _Alignas(8) uint8_t storage[POOL_BLOCKS][POOL_BLOCK_SIZE];
static bool in_use[POOL_BLOCKS];
static size_t free_stack[POOL_BLOCKS]; /* indices of free blocks; the top is handed out next */
static size_t free_top;

void pool_init(void) {
    for (size_t i = 0; i < POOL_BLOCKS; i++) {
        in_use[i] = false;
        free_stack[i] = POOL_BLOCKS - 1 - i; /* block 0 on top: nice, predictable order */
    }
    free_top = POOL_BLOCKS;
}

void *pool_alloc(void) {
    if (free_top == 0) {
        return NULL; /* exhausted: a known, testable outcome */
    }
    size_t i = free_stack[--free_top];
    in_use[i] = true;
    return storage[i];
}

bool pool_free(void *block) {
    if (block == NULL) {
        return false;
    }
    uintptr_t off = (uintptr_t)block - (uintptr_t)storage; /* wraps huge for pointers below the pool */
    if (off >= sizeof storage || off % POOL_BLOCK_SIZE != 0) {
        return false; /* foreign pointer, or the middle of a block */
    }
    size_t i = off / POOL_BLOCK_SIZE;
    if (!in_use[i]) {
        return false; /* double free */
    }
    in_use[i] = false;
    free_stack[free_top++] = i;
    return true;
}

size_t pool_free_count(void) {
    return free_top;
}
