#include <stddef.h>

TEST(finds_the_longest_streak_in_the_middle) {
    int week[] = {9000, 8000, 3000, 8500, 12000, 9100, 2000};
    EXPECT_EQ(longest_goal_streak(week, 7, 8000), 3);
}

TEST(no_day_reaching_the_goal_gives_zero) {
    int lazy[] = {5000, 4000, 7999};
    EXPECT_EQ(longest_goal_streak(lazy, 3, 8000), 0);
}

TEST(no_days_gives_zero) {
    int none[] = {9000};
    EXPECT_EQ(longest_goal_streak(none, 0, 8000), 0);
}

TEST(a_streak_running_to_the_last_day_counts) {
    int recent[] = {9000, 9500, 1200, 8000, 8800, 10000, 9100, 8300};
    EXPECT_EQ(longest_goal_streak(recent, 8, 8000), 5);
}

TEST(every_day_on_target_is_one_long_streak) {
    int perfect[] = {8000, 8001, 15000, 9999};
    EXPECT_EQ(longest_goal_streak(perfect, 4, 8000), 4);
}

TEST(a_single_good_day_is_a_streak_of_one) {
    int one[] = {10000};
    EXPECT_EQ(longest_goal_streak(one, 1, 8000), 1);
}
