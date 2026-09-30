#include <stddef.h>
#include <stdlib.h>

TEST(points_at_the_first_negative_balance) {
    int balances[] = {1500, 0, -250, 800, -40};
    int *p = find_first_negative(balances, 5);
    EXPECT_PTR_EQ(p, &balances[2]);
    EXPECT_EQ(p - balances, 2);
}

TEST(caller_can_write_through_the_result) {
    int balances[] = {10, -99, 20};
    int *p = find_first_negative(balances, 3);
    EXPECT_NOT_NULL(p);
    *p = 0;
    int expected[] = {10, 0, 20};
    EXPECT_INT_ARRAY_EQ(balances, expected, 3);
}

TEST(finds_a_negative_at_the_very_start) {
    int balances[] = {-1, -2};
    EXPECT_PTR_EQ(find_first_negative(balances, 2), &balances[0]);
}

TEST(returns_null_when_everything_is_fine) {
    int *balances = malloc(3 * sizeof *balances);
    balances[0] = 0;
    balances[1] = 5;
    balances[2] = 7;
    int *p = find_first_negative(balances, 3);
    free(balances);
    EXPECT_NULL(p);
}

TEST(only_looks_at_the_first_count_elements) {
    int balances[] = {100, 200, -300};
    EXPECT_NULL(find_first_negative(balances, 2));
}

TEST(empty_or_null_array_returns_null) {
    int balances[] = {-5};
    EXPECT_NULL(find_first_negative(balances, 0));
    EXPECT_NULL(find_first_negative(NULL, 4));
}
