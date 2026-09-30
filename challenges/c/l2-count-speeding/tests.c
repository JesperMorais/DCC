#include <stddef.h>

TEST(counts_cars_over_the_limit) {
    int cars[] = {48, 52, 50, 67, 31};
    EXPECT_EQ(count_speeding(cars, 5, 50), 2);
}

TEST(exactly_at_the_limit_is_not_speeding) {
    int cars[] = {50, 50, 50};
    EXPECT_EQ(count_speeding(cars, 3, 50), 0);
}

TEST(nobody_speeds_on_the_motorway) {
    int cars[] = {48, 52, 50, 67, 31};
    EXPECT_EQ(count_speeding(cars, 5, 70), 0);
}

TEST(everybody_speeds_in_a_school_zone) {
    int cars[] = {48, 52, 50, 67, 31};
    EXPECT_EQ(count_speeding(cars, 5, 30), 5);
}

TEST(only_the_first_count_cars) {
    int cars[] = {90, 40, 95, 99};
    EXPECT_EQ(count_speeding(cars, 3, 50), 2);
}

TEST(no_cars_means_no_speeders) {
    int none[] = {0};
    EXPECT_EQ(count_speeding(none, 0, 50), 0);
}
