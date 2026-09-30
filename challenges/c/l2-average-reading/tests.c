#include <stddef.h>

TEST(a_warm_day_keeps_the_half_degree) {
    int day[] = {20, 21, 22, 23};
    EXPECT_NEAR(average_reading(day, 4), 21.5, 1e-9);
}

TEST(a_whole_number_average) {
    int readings[] = {18, 20, 22};
    EXPECT_NEAR(average_reading(readings, 3), 20.0, 1e-9);
}

TEST(a_single_reading_is_its_own_average) {
    int one[] = {17};
    EXPECT_NEAR(average_reading(one, 1), 17.0, 1e-9);
}

TEST(a_cold_night_goes_negative) {
    int night[] = {-4, -5};
    EXPECT_NEAR(average_reading(night, 2), -4.5, 1e-9);
}

TEST(thirds_are_not_rounded_away) {
    int readings[] = {1, 1, 2};
    EXPECT_NEAR(average_reading(readings, 3), 4.0 / 3.0, 1e-9);
}

TEST(no_readings_gives_zero) {
    int none[] = {0};
    EXPECT_NEAR(average_reading(none, 0), 0.0, 1e-9);
}
