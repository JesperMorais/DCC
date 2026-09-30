TEST(best_round_is_first) {
    EXPECT_EQ(max_of_three(50, 10, 20), 50);
}

TEST(best_round_is_in_the_middle) {
    EXPECT_EQ(max_of_three(12, 40, 7), 40);
}

TEST(best_round_is_last) {
    EXPECT_EQ(max_of_three(3, 8, 15), 15);
}

TEST(all_rounds_equal) {
    EXPECT_EQ(max_of_three(5, 5, 5), 5);
}

TEST(ties_for_the_top) {
    EXPECT_EQ(max_of_three(9, 2, 9), 9);
}

TEST(all_negative_picks_the_least_bad) {
    EXPECT_EQ(max_of_three(-8, -3, -20), -3);
}
