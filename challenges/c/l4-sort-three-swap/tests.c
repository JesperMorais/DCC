TEST(swap_exchanges_two_values) {
    int x = 3, y = 7;
    swap_ints(&x, &y);
    EXPECT_EQ(x, 7);
    EXPECT_EQ(y, 3);
}

TEST(swapping_a_value_with_itself_changes_nothing) {
    int x = 5;
    swap_ints(&x, &x);
    EXPECT_EQ(x, 5);
}

TEST(swap_works_on_array_elements) {
    int times[] = {10, 20, 30};
    swap_ints(&times[0], &times[2]);
    int expected[] = {30, 20, 10};
    EXPECT_INT_ARRAY_EQ(times, expected, 3);
}

TEST(sorts_three_values) {
    int a = 58, b = 51, c = 64;
    sort_three(&a, &b, &c);
    EXPECT_EQ(a, 51);
    EXPECT_EQ(b, 58);
    EXPECT_EQ(c, 64);
}

TEST(sorts_every_ordering) {
    int orders[6][3] = {{1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1}};
    for (int k = 0; k < 6; k++) {
        sort_three(&orders[k][0], &orders[k][1], &orders[k][2]);
        int expected[] = {1, 2, 3};
        EXPECT_INT_ARRAY_EQ(orders[k], expected, 3);
    }
}

TEST(handles_duplicates_and_negatives) {
    int a = 4, b = -2, c = 4;
    sort_three(&a, &b, &c);
    EXPECT_EQ(a, -2);
    EXPECT_EQ(b, 4);
    EXPECT_EQ(c, 4);
}
