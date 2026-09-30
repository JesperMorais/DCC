#include <stddef.h>

TEST(fridge_readings) {
    int fridge[] = {4, 7, 3};
    EXPECT_EQ(highest_reading(fridge, 3), 7);
}

TEST(all_negative_freezer_readings) {
    int freezer[] = {-18, -21, -15, -19};
    EXPECT_EQ(highest_reading(freezer, 4), -15);
}

TEST(warmest_is_the_first_reading) {
    int readings[] = {9, 2, 5, 1};
    EXPECT_EQ(highest_reading(readings, 4), 9);
}

TEST(warmest_is_the_last_reading) {
    int readings[] = {-20, -19, -18, -2};
    EXPECT_EQ(highest_reading(readings, 4), -2);
}

TEST(a_single_reading) {
    int one[] = {-22};
    EXPECT_EQ(highest_reading(one, 1), -22);
}

TEST(only_the_first_count_readings) {
    int readings[] = {-18, -16, -17, 25};
    EXPECT_EQ(highest_reading(readings, 3), -16);
}
