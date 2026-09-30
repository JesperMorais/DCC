TEST(ninety_five_is_an_a) {
    EXPECT_EQ(grade_letter(95), 'A');
}

TEST(a_perfect_hundred_is_an_a) {
    EXPECT_EQ(grade_letter(100), 'A');
}

TEST(exactly_eighty_is_a_b) {
    EXPECT_EQ(grade_letter(80), 'B');
    EXPECT_EQ(grade_letter(89), 'B');
}

TEST(seventy_two_is_a_c) {
    EXPECT_EQ(grade_letter(72), 'C');
}

TEST(sixty_is_a_d_but_fifty_nine_is_an_f) {
    EXPECT_EQ(grade_letter(60), 'D');
    EXPECT_EQ(grade_letter(59), 'F');
}

TEST(low_scores_are_an_f) {
    EXPECT_EQ(grade_letter(12), 'F');
    EXPECT_EQ(grade_letter(0), 'F');
}
