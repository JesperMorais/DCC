TEST(two_minutes_five_seconds) {
    EXPECT_EQ(microwave_display(125), 205);
}

TEST(ninety_seconds_is_one_thirty) {
    EXPECT_EQ(microwave_display(90), 130);
}

TEST(under_a_minute_shows_just_seconds) {
    EXPECT_EQ(microwave_display(45), 45);
    EXPECT_EQ(microwave_display(59), 59);
}

TEST(exactly_one_minute) {
    EXPECT_EQ(microwave_display(60), 100);
}

TEST(ten_whole_minutes) {
    EXPECT_EQ(microwave_display(600), 1000);
}

TEST(zero_seconds) {
    EXPECT_EQ(microwave_display(0), 0);
}
