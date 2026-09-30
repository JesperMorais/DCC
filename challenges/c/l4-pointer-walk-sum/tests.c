#include <stdlib.h>

TEST(sums_readings_above_the_limit) {
    int watts[] = {120, 900, 450, 1300, 80};
    EXPECT_EQ(sum_above(watts, watts + 5, 400), 2650);
}

TEST(limit_itself_is_not_included) {
    int watts[] = {400, 401, 399};
    EXPECT_EQ(sum_above(watts, watts + 3, 400), 401);
}

TEST(works_on_a_slice_in_the_middle) {
    int watts[] = {5000, 900, 450, 5000};
    EXPECT_EQ(sum_above(watts + 1, watts + 3, 500), 900);
}

TEST(empty_range_is_zero) {
    int watts[] = {1000};
    EXPECT_EQ(sum_above(watts, watts, 0), 0);
}

TEST(negative_limit_and_readings) {
    int deltas[] = {-5, -1, 0, 3};
    EXPECT_EQ(sum_above(deltas, deltas + 4, -2), 2);
}

TEST(never_dereferences_the_end_pointer) {
    int *watts = malloc(3 * sizeof *watts);
    watts[0] = 700;
    watts[1] = 800;
    watts[2] = 900;
    long total = sum_above(watts, watts + 3, 750);
    free(watts);
    EXPECT_EQ(total, 1700);
}
