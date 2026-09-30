#include <limits.h>
#include <stdbool.h>

TEST(ordinary_sums_and_differences) {
    int r = 0;
    EXPECT_TRUE(checked_add(2, 3, &r));
    EXPECT_EQ(r, 5);
    EXPECT_TRUE(checked_add(-7, 4, &r));
    EXPECT_EQ(r, -3);
    EXPECT_TRUE(checked_sub(10, 25, &r));
    EXPECT_EQ(r, -15);
    EXPECT_TRUE(checked_add(2000000000, 100000000, &r));
    EXPECT_EQ(r, 2100000000);
}

TEST(results_exactly_at_the_limits_fit) {
    int r = 0;
    EXPECT_TRUE(checked_add(INT_MAX - 1, 1, &r));
    EXPECT_EQ(r, INT_MAX);
    EXPECT_TRUE(checked_add(INT_MIN + 1, -1, &r));
    EXPECT_EQ(r, INT_MIN);
    EXPECT_TRUE(checked_add(INT_MAX, INT_MIN, &r));
    EXPECT_EQ(r, -1);
    EXPECT_TRUE(checked_add(INT_MIN, 0, &r));
    EXPECT_EQ(r, INT_MIN);
}

TEST(add_overflow_is_refused_and_out_is_untouched) {
    int r = 12345;
    EXPECT_FALSE(checked_add(INT_MAX, 1, &r));
    EXPECT_FALSE(checked_add(1, INT_MAX, &r));
    EXPECT_FALSE(checked_add(INT_MAX, INT_MAX, &r));
    EXPECT_FALSE(checked_add(INT_MIN, -1, &r));
    EXPECT_FALSE(checked_add(INT_MIN, INT_MIN, &r));
    EXPECT_EQ(r, 12345);
}

TEST(sub_near_the_limits) {
    int r = 0;
    EXPECT_TRUE(checked_sub(-1, INT_MIN, &r));
    EXPECT_EQ(r, INT_MAX);
    EXPECT_TRUE(checked_sub(INT_MIN, INT_MIN, &r));
    EXPECT_EQ(r, 0);
    EXPECT_TRUE(checked_sub(INT_MAX, INT_MAX, &r));
    EXPECT_EQ(r, 0);
}

TEST(sub_overflow_is_refused_and_out_is_untouched) {
    int r = 777;
    EXPECT_FALSE(checked_sub(0, INT_MIN, &r));
    EXPECT_FALSE(checked_sub(INT_MAX, -1, &r));
    EXPECT_FALSE(checked_sub(INT_MIN, 1, &r));
    EXPECT_FALSE(checked_sub(INT_MIN, INT_MAX, &r));
    EXPECT_EQ(r, 777);
}
