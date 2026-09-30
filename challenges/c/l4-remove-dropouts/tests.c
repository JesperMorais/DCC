#include <stddef.h>
#include <stdlib.h>

TEST(removes_sentinels_and_keeps_order) {
    int temps[] = {21, -999, 22, -999, -999, 23};
    size_t n = remove_sentinel(temps, 6, -999);
    EXPECT_UEQ(n, 3);
    int expected[] = {21, 22, 23};
    EXPECT_INT_ARRAY_EQ(temps, expected, 3);
}

TEST(nothing_to_remove_leaves_the_array_alone) {
    int temps[] = {5, 6, 7};
    EXPECT_UEQ(remove_sentinel(temps, 3, -999), 3);
    int expected[] = {5, 6, 7};
    EXPECT_INT_ARRAY_EQ(temps, expected, 3);
}

TEST(everything_removed_gives_zero) {
    int temps[] = {0, 0, 0, 0};
    EXPECT_UEQ(remove_sentinel(temps, 4, 0), 0);
}

TEST(sentinels_at_both_ends) {
    int *temps = malloc(5 * sizeof *temps);
    int input[] = {-1, 18, 19, -1, -1};
    for (int i = 0; i < 5; i++) temps[i] = input[i];
    size_t n = remove_sentinel(temps, 5, -1);
    int got[2] = {temps[0], temps[1]};
    free(temps);
    EXPECT_UEQ(n, 2);
    int expected[] = {18, 19};
    EXPECT_INT_ARRAY_EQ(got, expected, 2);
}

TEST(only_touches_the_first_count_elements) {
    int temps[] = {-999, 4, -999, 99};
    EXPECT_UEQ(remove_sentinel(temps, 3, -999), 1);
    EXPECT_EQ(temps[0], 4);
    EXPECT_EQ(temps[3], 99);
}

TEST(empty_and_null_arrays) {
    int temps[] = {-999};
    EXPECT_UEQ(remove_sentinel(temps, 0, -999), 0);
    EXPECT_EQ(temps[0], -999);
    EXPECT_UEQ(remove_sentinel(NULL, 5, -999), 0);
}
