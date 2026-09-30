TEST(water_boils_at_212) {
    EXPECT_NEAR(celsius_to_fahrenheit(100.0), 212.0, 1e-9);
}

TEST(water_freezes_at_32) {
    EXPECT_NEAR(celsius_to_fahrenheit(0.0), 32.0, 1e-9);
}

TEST(body_temperature_keeps_its_decimal) {
    EXPECT_NEAR(celsius_to_fahrenheit(37.0), 98.6, 1e-9);
}

TEST(minus_forty_is_the_same_on_both_scales) {
    EXPECT_NEAR(celsius_to_fahrenheit(-40.0), -40.0, 1e-9);
}

TEST(fractional_celsius) {
    EXPECT_NEAR(celsius_to_fahrenheit(21.5), 70.7, 1e-9);
}
