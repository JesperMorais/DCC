#include <stdbool.h>

TEST(hands_out_the_lowest_free_blocks_first) {
    BlockPool pool;
    pool_init(&pool);
    EXPECT_EQ(pool_alloc(&pool), 0);
    EXPECT_EQ(pool_alloc(&pool), 1);
    EXPECT_EQ(pool_alloc(&pool), 2);
    EXPECT_TRUE(pool_is_used(&pool, 1));
    EXPECT_FALSE(pool_is_used(&pool, 3));
}

TEST(a_freed_block_is_reused) {
    BlockPool pool;
    pool_init(&pool);
    pool_alloc(&pool);
    pool_alloc(&pool);
    EXPECT_TRUE(pool_free(&pool, 0));
    EXPECT_FALSE(pool_is_used(&pool, 0));
    EXPECT_EQ(pool_alloc(&pool), 0);
}

TEST(freeing_twice_or_out_of_range_is_rejected) {
    BlockPool pool;
    pool_init(&pool);
    pool_alloc(&pool);
    EXPECT_TRUE(pool_free(&pool, 0));
    EXPECT_FALSE(pool_free(&pool, 0));
    EXPECT_FALSE(pool_free(&pool, POOL_BLOCKS));
    EXPECT_FALSE(pool_is_used(&pool, POOL_BLOCKS));
}

TEST(allocates_past_the_first_word) {
    BlockPool pool;
    pool_init(&pool);
    for (int i = 0; i < 40; i++) EXPECT_EQ(pool_alloc(&pool), i);
    EXPECT_TRUE(pool_is_used(&pool, 31));
    EXPECT_TRUE(pool_is_used(&pool, 32));
}

TEST(every_block_once_then_none) {
    BlockPool pool;
    pool_init(&pool);
    for (int i = 0; i < POOL_BLOCKS; i++) EXPECT_EQ(pool_alloc(&pool), i);
    EXPECT_EQ(pool_alloc(&pool), -1);
}

TEST(the_top_bit_of_each_word_can_be_freed_and_reused) {
    BlockPool pool;
    pool_init(&pool);
    for (int i = 0; i < POOL_BLOCKS; i++) pool_alloc(&pool);
    EXPECT_TRUE(pool_free(&pool, 63));
    EXPECT_TRUE(pool_free(&pool, 31));
    EXPECT_FALSE(pool_is_used(&pool, 31));
    EXPECT_TRUE(pool_is_used(&pool, 30));
    EXPECT_TRUE(pool_is_used(&pool, 32));
    EXPECT_EQ(pool_alloc(&pool), 31);
    EXPECT_EQ(pool_alloc(&pool), 63);
    EXPECT_EQ(pool_alloc(&pool), -1);
}
