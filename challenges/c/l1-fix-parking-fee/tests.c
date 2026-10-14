TEST(no_time_parked_is_free) {
    EXPECT_EQ(garage_parking_fee(0), 0);
}

TEST(short_visits_are_free) {
    EXPECT_EQ(garage_parking_fee(1), 0);
    EXPECT_EQ(garage_parking_fee(2), 0);
}

TEST(one_hour_past_the_free_time_costs_15) {
    EXPECT_EQ(garage_parking_fee(3), 15);
}

TEST(only_the_hours_after_the_free_time_are_charged) {
    EXPECT_EQ(garage_parking_fee(4), 30);
    EXPECT_EQ(garage_parking_fee(10), 120);
}
