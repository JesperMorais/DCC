#include <stddef.h>

TEST(a_full_week) {
    int week[] = {8000, 10250, 4300, 12000, 9100, 15600, 3000};
    EXPECT_EQ(total_steps(week, 7), 62250);
}

TEST(a_single_day) {
    int day[] = {4321};
    EXPECT_EQ(total_steps(day, 1), 4321);
}

TEST(only_the_first_count_days) {
    int week[] = {8000, 10250, 4300, 12000, 9100, 15600, 3000};
    EXPECT_EQ(total_steps(week, 2), 18250);
}

TEST(lazy_days_count_as_zero) {
    int days[] = {0, 5000, 0};
    EXPECT_EQ(total_steps(days, 3), 5000);
}

TEST(no_days_recorded_is_zero) {
    int none[] = {0};
    EXPECT_EQ(total_steps(none, 0), 0);
}
