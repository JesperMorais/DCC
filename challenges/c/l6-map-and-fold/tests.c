#include <stddef.h>

static int twice(int x) { return 2 * x; }
static int clamp_0_100(int x) { return x < 0 ? 0 : x > 100 ? 100 : x; }
static int add(int a, int b) { return a + b; }
static int larger(int a, int b) { return a > b ? a : b; }
static int shift_digits(int acc, int d) { return acc * 10 + d; }   // order-sensitive

static int calls;
static int seen[8];
static int record(int x) { if (calls < 8) seen[calls] = x; calls++; return x; }
static int explode(int a, int b) { (void)a; (void)b; calls++; return -1; }

TEST(map_applies_the_function_in_place) {
    int r[] = {1, 2, 3, -4};
    map_ints(r, 4, twice);
    int expected[] = {2, 4, 6, -8};
    EXPECT_INT_ARRAY_EQ(r, expected, 4);
}

TEST(map_works_with_any_callback) {
    int r[] = {-20, 50, 130, 100};
    map_ints(r, 4, clamp_0_100);
    int expected[] = {0, 50, 100, 100};
    EXPECT_INT_ARRAY_EQ(r, expected, 4);
}

TEST(map_calls_the_callback_once_per_element_in_order) {
    int r[] = {7, 8, 9, 10};
    calls = 0;
    map_ints(r, 3, record);   // only the first 3
    EXPECT_EQ(calls, 3);
    int expected[] = {7, 8, 9};
    EXPECT_INT_ARRAY_EQ(seen, expected, 3);
    EXPECT_EQ(r[3], 10);
}

TEST(fold_sums_and_finds_the_peak) {
    int r[] = {4, -2, 9, 3};
    EXPECT_EQ(fold_ints(r, 4, 0, add), 14);
    EXPECT_EQ(fold_ints(r, 4, r[0], larger), 9);
}

TEST(fold_goes_left_to_right_starting_from_init) {
    int digits[] = {1, 2, 3};
    EXPECT_EQ(fold_ints(digits, 3, 0, shift_digits), 123);
    EXPECT_EQ(fold_ints(digits, 3, 9, shift_digits), 9123);
}

TEST(empty_input_never_calls_the_callback) {
    int r[] = {5};
    calls = 0;
    map_ints(r, 0, record);
    EXPECT_EQ(fold_ints(r, 0, 42, explode), 42);
    EXPECT_EQ(calls, 0);
    EXPECT_EQ(r[0], 5);
}
