#include <stdbool.h>

TEST(monday_is_a_working_day) {
    EXPECT_TRUE(is_working_day(1));
}

TEST(midweek_days_are_working_days) {
    EXPECT_TRUE(is_working_day(2));
    EXPECT_TRUE(is_working_day(3));
    EXPECT_TRUE(is_working_day(4));
}

TEST(friday_is_a_working_day) {
    EXPECT_TRUE(is_working_day(5));
}

TEST(weekend_is_not) {
    EXPECT_FALSE(is_working_day(6));
    EXPECT_FALSE(is_working_day(7));
}

TEST(numbers_that_are_not_days_are_not_working_days) {
    EXPECT_FALSE(is_working_day(0));
    EXPECT_FALSE(is_working_day(8));
    EXPECT_FALSE(is_working_day(-1));
}
