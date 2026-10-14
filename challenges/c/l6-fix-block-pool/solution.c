#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POOL_BLOCKS 64
#define WORD_BITS 32

typedef struct {
    uint32_t used[POOL_BLOCKS / WORD_BITS];
    size_t free_count;
} BlockPool;

static uint32_t block_mask(size_t block) {
    return UINT32_C(1) << (block % WORD_BITS);
}

static uint32_t *block_word(BlockPool *pool, size_t block) {
    return &pool->used[block / WORD_BITS];
}

void pool_init(BlockPool *pool) {
    for (size_t w = 0; w < POOL_BLOCKS / WORD_BITS; w++) {
        pool->used[w] = 0;
    }
    pool->free_count = POOL_BLOCKS;
}

bool pool_is_used(const BlockPool *pool, size_t block) {
    return block < POOL_BLOCKS && (pool->used[block / WORD_BITS] & block_mask(block)) != 0;
}

int pool_alloc(BlockPool *pool) {
    if (pool->free_count == 0) {
        return -1;
    }
    for (size_t block = 0; block < POOL_BLOCKS; block++) {
        if (*block_word(pool, block) == UINT32_MAX) {
            block += WORD_BITS - 1;
            continue;
        }
        if (!pool_is_used(pool, block)) {
            *block_word(pool, block) |= block_mask(block);
            pool->free_count--;
            return (int)block;
        }
    }
    return -1;
}

bool pool_free(BlockPool *pool, size_t block) {
    if (!pool_is_used(pool, block)) {
        return false;
    }
    *block_word(pool, block) &= ~block_mask(block);
    pool->free_count++;
    return true;
}
