TEST(light_parcels_cost_49) {
    EXPECT_EQ(parcel_shipping_cost(400), 49);
    EXPECT_EQ(parcel_shipping_cost(1000), 49);
}

TEST(medium_parcels_cost_79) {
    EXPECT_EQ(parcel_shipping_cost(1001), 79);
    EXPECT_EQ(parcel_shipping_cost(3000), 79);
}

TEST(exactly_5000_grams_is_still_medium) {
    EXPECT_EQ(parcel_shipping_cost(5000), 79);
}

TEST(heavy_parcels_cost_129) {
    EXPECT_EQ(parcel_shipping_cost(5001), 129);
    EXPECT_EQ(parcel_shipping_cost(12000), 129);
}
