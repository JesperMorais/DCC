TEST(a_four_digit_year) {
    EXPECT_EQ(count_digits(2026), 4);
}

TEST(single_digit) {
    EXPECT_EQ(count_digits(7), 1);
}

TEST(zero_has_one_digit) {
    EXPECT_EQ(count_digits(0), 1);
}

TEST(powers_of_ten_are_one_digit_longer) {
    EXPECT_EQ(count_digits(9), 1);
    EXPECT_EQ(count_digits(10), 2);
    EXPECT_EQ(count_digits(1000000), 7);
}

TEST(minus_sign_is_not_a_digit) {
    EXPECT_EQ(count_digits(-4096), 4);
    EXPECT_EQ(count_digits(-5), 1);
}
