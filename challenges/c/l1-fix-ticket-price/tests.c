TEST(children_pay_60) {
    EXPECT_EQ(cinema_ticket_price(8), 60);
    EXPECT_EQ(cinema_ticket_price(0), 60);
}

TEST(twelve_is_no_longer_a_child) {
    EXPECT_EQ(cinema_ticket_price(11), 60);
    EXPECT_EQ(cinema_ticket_price(12), 120);
}

TEST(adults_pay_120) {
    EXPECT_EQ(cinema_ticket_price(30), 120);
    EXPECT_EQ(cinema_ticket_price(64), 120);
}

TEST(sixty_five_gets_the_senior_price) {
    EXPECT_EQ(cinema_ticket_price(65), 80);
}

TEST(older_seniors_pay_80) {
    EXPECT_EQ(cinema_ticket_price(70), 80);
    EXPECT_EQ(cinema_ticket_price(98), 80);
}
