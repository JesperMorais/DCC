#include <stddef.h>

TEST(counts_the_rises_in_a_day) {
    int today[] = {12, 14, 13, 15, 15, 18};
    EXPECT_EQ(count_rises(today, 6), 3);
}

TEST(steady_climb_rises_every_time) {
    int morning[] = {5, 6, 7, 8};
    EXPECT_EQ(count_rises(morning, 4), 3);
}

TEST(falling_or_flat_is_never_a_rise) {
    int evening[] = {20, 18, 18, 11};
    EXPECT_EQ(count_rises(evening, 4), 0);
}

TEST(a_single_reading_has_no_rises) {
    int one[] = {9};
    EXPECT_EQ(count_rises(one, 1), 0);
}

TEST(no_readings_has_no_rises) {
    int none[] = {0};
    EXPECT_EQ(count_rises(none, 0), 0);
}

TEST(tomorrow_is_not_part_of_today) {
    int readings[] = {3, 1, 2, 99};
    EXPECT_EQ(count_rises(readings, 3), 1);
}
