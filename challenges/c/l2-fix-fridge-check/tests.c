#include <stdbool.h>
#include <stddef.h>

TEST(a_normal_day_is_in_range) {
    int monday[] = {3, 4, 5, 4};
    EXPECT_TRUE(all_readings_in_range(monday, 4, 0, 8));
}

TEST(a_bad_first_reading_fails_the_day) {
    int friday[] = {-1, 4, 5};
    EXPECT_FALSE(all_readings_in_range(friday, 3, 0, 8));
}

TEST(a_bad_reading_in_the_middle_fails_the_day) {
    int tuesday[] = {4, 5, 9, 6};
    EXPECT_FALSE(all_readings_in_range(tuesday, 4, 0, 8));
}

TEST(a_bad_last_reading_fails_the_day) {
    int sunday[] = {2, 3, 3, 12};
    EXPECT_FALSE(all_readings_in_range(sunday, 4, 0, 8));
}

TEST(the_limits_themselves_are_allowed) {
    int edges[] = {0, 8, 0, 8};
    EXPECT_TRUE(all_readings_in_range(edges, 4, 0, 8));
}

TEST(no_readings_means_nothing_is_wrong) {
    int none[] = {99};
    EXPECT_TRUE(all_readings_in_range(none, 0, 0, 8));
}
