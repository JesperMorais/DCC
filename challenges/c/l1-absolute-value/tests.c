TEST(positive_numbers_stay_the_same) {
    EXPECT_EQ(distance_from_zero(7), 7);
    EXPECT_EQ(distance_from_zero(1), 1);
}

TEST(negative_numbers_lose_their_sign) {
    EXPECT_EQ(distance_from_zero(-7), 7);
    EXPECT_EQ(distance_from_zero(-250), 250);
}

TEST(zero_is_zero) {
    EXPECT_EQ(distance_from_zero(0), 0);
}
