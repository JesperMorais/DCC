#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

TEST(a_fresh_pool_has_every_block_free) {
    pool_init();
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS);
}

TEST(blocks_are_distinct_aligned_and_do_not_overlap) {
    pool_init();
    uint8_t *b[POOL_BLOCKS];
    for (int i = 0; i < POOL_BLOCKS; i++) {
        b[i] = pool_alloc();
        EXPECT_NOT_NULL(b[i]);
        EXPECT_EQ((uintptr_t)b[i] % 8, 0);                 /* can hold a uint64_t */
        memset(b[i], 0xA0 + i, POOL_BLOCK_SIZE);           /* fill the whole block */
        EXPECT_EQ(pool_free_count(), POOL_BLOCKS - 1 - i);
    }
    for (int i = 0; i < POOL_BLOCKS; i++)
        for (int k = 0; k < POOL_BLOCK_SIZE; k++)
            EXPECT_EQ(b[i][k], 0xA0 + i);                  /* nobody overwrote anybody */
    for (int i = 0; i < POOL_BLOCKS; i++) EXPECT_TRUE(pool_free(b[i]));
}

TEST(an_exhausted_pool_returns_null_and_recovers_after_a_free) {
    pool_init();
    void *b[POOL_BLOCKS];
    for (int i = 0; i < POOL_BLOCKS; i++) b[i] = pool_alloc();
    EXPECT_NULL(pool_alloc());
    EXPECT_NULL(pool_alloc());
    EXPECT_EQ(pool_free_count(), 0);
    EXPECT_TRUE(pool_free(b[3]));
    EXPECT_EQ(pool_free_count(), 1);
    EXPECT_PTR_EQ(pool_alloc(), b[3]);                     /* the only free block */
    EXPECT_NULL(pool_alloc());
}

TEST(a_double_free_is_rejected_and_does_not_corrupt_the_pool) {
    pool_init();
    void *a = pool_alloc();
    EXPECT_TRUE(pool_free(a));
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS);
    EXPECT_FALSE(pool_free(a));                            /* second free of the same block */
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS);
    /* if the block went on the free list twice, two owners would now share it */
    void *b[POOL_BLOCKS];
    for (int i = 0; i < POOL_BLOCKS; i++) {
        b[i] = pool_alloc();
        EXPECT_NOT_NULL(b[i]);
        for (int k = 0; k < i; k++) EXPECT_TRUE(b[i] != b[k]);
    }
    EXPECT_NULL(pool_alloc());
}

TEST(null_foreign_and_interior_pointers_are_rejected) {
    pool_init();
    uint8_t *b = pool_alloc();
    uint8_t on_the_stack[POOL_BLOCK_SIZE];
    static uint8_t some_global[POOL_BLOCK_SIZE];
    EXPECT_FALSE(pool_free(NULL));
    EXPECT_FALSE(pool_free(on_the_stack));
    EXPECT_FALSE(pool_free(some_global));
    EXPECT_FALSE(pool_free(b + 4));                        /* inside a block, not its start */
    EXPECT_FALSE(pool_free(b + POOL_BLOCK_SIZE - 1));
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS - 1);         /* nothing changed */
    EXPECT_TRUE(pool_free(b));                             /* the real pointer still works */
}

TEST(pool_init_resets_everything) {
    pool_init();
    for (int i = 0; i < 5; i++) EXPECT_NOT_NULL(pool_alloc());
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS - 5);
    pool_init();
    EXPECT_EQ(pool_free_count(), POOL_BLOCKS);
    for (int i = 0; i < POOL_BLOCKS; i++) EXPECT_NOT_NULL(pool_alloc());
    EXPECT_NULL(pool_alloc());
}

TEST(survives_a_long_alloc_free_churn_without_handing_out_live_blocks) {
    pool_init();
    uint8_t *held[POOL_BLOCKS] = {0};
    unsigned seed = 12345;
    for (int round = 0; round < 2000; round++) {
        seed = seed * 1103515245u + 12345u;
        int slot = (int)((seed >> 16) % POOL_BLOCKS);
        if (held[slot]) {
            for (int k = 0; k < POOL_BLOCK_SIZE; k++) EXPECT_EQ(held[slot][k], (uint8_t)slot);
            EXPECT_TRUE(pool_free(held[slot]));
            held[slot] = NULL;
        } else {
            held[slot] = pool_alloc();
            EXPECT_NOT_NULL(held[slot]);
            memset(held[slot], slot, POOL_BLOCK_SIZE);
        }
        size_t live = 0;
        for (int i = 0; i < POOL_BLOCKS; i++) live += held[i] != NULL;
        EXPECT_EQ(pool_free_count(), POOL_BLOCKS - live);
    }
}
