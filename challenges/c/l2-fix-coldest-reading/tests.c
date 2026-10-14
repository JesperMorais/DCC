#include <stddef.h>

TEST(finds_the_coldest_winter_night) {
    int january[] = {-3, -11, -7, -2};
    EXPECT_EQ(coldest_reading(january, 4), -11);
}

TEST(a_mixed_week_with_one_frost) {
    int march[] = {4, -1, 6};
    EXPECT_EQ(coldest_reading(march, 3), -1);
}

TEST(finds_the_coldest_summer_night) {
    int july[] = {17, 14, 19, 16};
    EXPECT_EQ(coldest_reading(july, 4), 14);
}

TEST(a_single_reading_is_the_coldest) {
    int one[] = {21};
    EXPECT_EQ(coldest_reading(one, 1), 21);
}

TEST(the_coldest_can_be_the_last_reading) {
    int autumn[] = {9, 8, 6, 3};
    EXPECT_EQ(coldest_reading(autumn, 4), 3);
}
