#include <stdbool.h>

TEST(divisible_by_four_is_a_leap_year) {
    EXPECT_TRUE(is_leap_year(2024));
    EXPECT_TRUE(is_leap_year(1996));
}

TEST(ordinary_years_are_not) {
    EXPECT_FALSE(is_leap_year(2023));
    EXPECT_FALSE(is_leap_year(2026));
}

TEST(centuries_are_not_leap_years) {
    EXPECT_FALSE(is_leap_year(1900));
    EXPECT_FALSE(is_leap_year(2100));
}

TEST(unless_divisible_by_four_hundred) {
    EXPECT_TRUE(is_leap_year(2000));
    EXPECT_TRUE(is_leap_year(2400));
}
