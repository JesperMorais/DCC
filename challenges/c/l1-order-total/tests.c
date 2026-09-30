TEST(three_items_plus_shipping) {
    EXPECT_EQ(order_total(3, 250, 49), 799);
}

TEST(free_shipping_means_just_the_items) {
    EXPECT_EQ(order_total(1, 1999, 0), 1999);
}

TEST(nothing_ordered_still_pays_shipping) {
    EXPECT_EQ(order_total(0, 500, 49), 49);
}

TEST(a_bigger_order) {
    EXPECT_EQ(order_total(12, 1500, 99), 18099);
}
