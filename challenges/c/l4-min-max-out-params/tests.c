#include <stdbool.h>
#include <stddef.h>

TEST(finds_both_ends) {
    int temps[] = {3, -2, 9, 4};
    int lo = 0, hi = 0;
    EXPECT_TRUE(min_max(temps, 4, &lo, &hi));
    EXPECT_EQ(lo, -2);
    EXPECT_EQ(hi, 9);
}

TEST(single_element_is_both_min_and_max) {
    int one[] = {42};
    int lo = 0, hi = 0;
    EXPECT_TRUE(min_max(one, 1, &lo, &hi));
    EXPECT_EQ(lo, 42);
    EXPECT_EQ(hi, 42);
}

TEST(all_negative_values) {
    int cold[] = {-8, -3, -15, -4};
    int lo = 0, hi = 0;
    EXPECT_TRUE(min_max(cold, 4, &lo, &hi));
    EXPECT_EQ(lo, -15);
    EXPECT_EQ(hi, -3);
}

TEST(empty_array_returns_false_and_leaves_outputs_alone) {
    int lo = 111, hi = 222;
    int dummy[] = {0};
    EXPECT_FALSE(min_max(dummy, 0, &lo, &hi));
    EXPECT_EQ(lo, 111);
    EXPECT_EQ(hi, 222);
}

TEST(null_array_returns_false) {
    int lo = 111, hi = 222;
    EXPECT_FALSE(min_max(NULL, 3, &lo, &hi));
    EXPECT_EQ(lo, 111);
    EXPECT_EQ(hi, 222);
}

TEST(does_not_read_past_count) {
    int values[] = {5, 1, 7, -100};
    int lo = 0, hi = 0;
    EXPECT_TRUE(min_max(values, 3, &lo, &hi));
    EXPECT_EQ(lo, 1);
    EXPECT_EQ(hi, 7);
}
